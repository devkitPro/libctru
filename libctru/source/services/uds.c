#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/uds.h>
#include <3ds/services/cfgu.h>
#include <3ds/services/ndm.h>
#include <3ds/ipc.h>
#include <3ds/util/utf.h>

Handle __uds_servhandle;
static int __uds_refcount;

u32 *__uds_sharedmem_addr;
static u32 __uds_sharedmem_size;
static Handle __uds_sharedmem_handle;

static Handle __uds_connectionstatus_event;

static u32 bind_allocbitmask;

static Result uds_Initialize(u32 sharedmem_size, const uint8_t *username);
static Result udsipc_InitializeWithVersion(udsNodeInfo *nodeinfo, Handle sharedmem_handle, u32 sharedmem_size, Handle *eventhandle);
static Result udsipc_Shutdown(void);

static Result udsipc_BeginHostingNetwork(udsNetworkStruct *network, void* passphrase, size_t passphrase_size);
static Result udsipc_SetProbeResponseParam(u32 oui, s8 data);

static Result udsipc_Bind(udsBindContext *bindcontext, u32 input0, u8 input1, u16 NetworkNodeID);
static Result udsipc_Unbind(udsBindContext *bindcontext);

Result udsInit(u32 sharedmem_size, const uint8_t *username)
{
	Result ret=0;
	u32 ndm_state = 0;

	if (AtomicPostIncrement(&__uds_refcount)) return 0;

	ret = ndmuInit();
	if(R_SUCCEEDED(ret))
	{
		ndm_state = 1;
		ret = ndmuEnterExclusiveState(EXCLUSIVE_STATE_LOCAL_COMMUNICATIONS);
		if(R_SUCCEEDED(ret))
		{
			ndm_state = 2;
		}
	}

	if(R_SUCCEEDED(ret))
	{
		ret = srvGetServiceHandle(&__uds_servhandle, "nwm::UDS");
		if(R_SUCCEEDED(ret))
		{
			ret = uds_Initialize(sharedmem_size, username);
			if (R_FAILED(ret))
			{
				svcCloseHandle(__uds_servhandle);
				__uds_servhandle = 0;
			}
		}
	}

	if (R_FAILED(ret))
	{
		if(ndm_state)
		{
			if(ndm_state==2)ndmuLeaveExclusiveState();
			ndmuExit();
		}

		AtomicDecrement(&__uds_refcount);
	}

	bind_allocbitmask = 0;

	return ret;
}

void udsExit(void)
{
	if (AtomicDecrement(&__uds_refcount)) return;

	udsipc_Shutdown();

	svcCloseHandle(__uds_servhandle);
	__uds_servhandle = 0;

	svcCloseHandle(__uds_sharedmem_handle);
	__uds_sharedmem_handle = 0;
	__uds_sharedmem_size = 0;

	free(__uds_sharedmem_addr);
	__uds_sharedmem_addr = NULL;

	svcCloseHandle(__uds_connectionstatus_event);
	__uds_connectionstatus_event = 0;

	ndmuLeaveExclusiveState();
	ndmuExit();
}

Result udsGenerateNodeInfo(udsNodeInfo *nodeinfo, const uint8_t *username)
{
	Result ret=0;
	ssize_t units=0;
	size_t len;
	u8 tmp[0x1c];

	memset(nodeinfo, 0, sizeof(udsNodeInfo));
	memset(tmp, 0, sizeof(tmp));

	ret = cfguInit();
	if (R_FAILED(ret))return ret;

	ret = CFGU_GetConfigInfoBlk2(sizeof(nodeinfo->uds_friendcodeseed), 0x00090000, (u8*)&nodeinfo->uds_friendcodeseed);
	if (R_FAILED(ret))
	{
		cfguExit();
		return ret;
	}

	ret = CFGU_GetConfigInfoBlk2(sizeof(tmp), 0x000A0000, tmp);
	if (R_FAILED(ret))
	{
		cfguExit();
		return ret;
	}

	memcpy(nodeinfo->usercfg, tmp, sizeof(nodeinfo->usercfg));

	if(username)
	{
		len = 10;

		memset(nodeinfo->usercfg, 0, len*2);

		units = utf8_to_utf16((uint16_t*)nodeinfo->usercfg, username, len);

		if(units < 0 || units > len)ret = -2;
	}

	cfguExit();

	return ret;
}

Result udsGetNodeInfoUsername(udsNodeInfo *nodeinfo, uint8_t *username)
{
	ssize_t units=0;
	size_t len = 10;

	units = utf16_to_utf8(username, (uint16_t*)nodeinfo->usercfg, len);

	if(units < 0 || units > len)return -2;
	return 0;
}

void udsGenerateDefaultNetworkStruct(udsNetworkStruct *network, u32 wlancommID, u8 id8, u8 max_nodes)
{
	u8 oui_value[3] = {0x00, 0x1f, 0x32};

	memset(network, 0, sizeof(udsNetworkStruct));

	network->initialized_flag = 1;

	memcpy(network->oui_value, oui_value, 3);
	network->oui_type = 21;

	network->wlancommID = htonl(wlancommID);
	network->id8 = id8;

	network->attributes = UDSNETATTR_Default;

	if(max_nodes > UDS_MAXNODES)max_nodes = UDS_MAXNODES;
	network->max_nodes = max_nodes;

	network->unk_x1f = 1;
}

static Result uds_Initialize(u32 sharedmem_size, const uint8_t *username)
{
	Result ret=0;
	udsNodeInfo nodeinfo;

	ret = udsGenerateNodeInfo(&nodeinfo, username);
	if (R_FAILED(ret))return ret;

	__uds_sharedmem_size = sharedmem_size;
	__uds_sharedmem_handle = 0;

	__uds_sharedmem_addr = memalign(0x1000, __uds_sharedmem_size);
	if(__uds_sharedmem_addr==NULL)ret = -1;

	if (R_SUCCEEDED(ret))
	{
		memset(__uds_sharedmem_addr, 0, __uds_sharedmem_size);
		ret = svcCreateMemoryBlock(&__uds_sharedmem_handle, (u32)__uds_sharedmem_addr, __uds_sharedmem_size, 0x0, MEMPERM_READ | MEMPERM_WRITE);
	}

	if (R_SUCCEEDED(ret))ret = udsipc_InitializeWithVersion(&nodeinfo, __uds_sharedmem_handle, __uds_sharedmem_size, &__uds_connectionstatus_event);

	if (R_FAILED(ret) && __uds_sharedmem_handle)
	{
		svcCloseHandle(__uds_sharedmem_handle);
		__uds_sharedmem_handle = 0;
		__uds_sharedmem_size = 0;
	}

	if(R_FAILED(ret) && __uds_sharedmem_addr)
	{
		free(__uds_sharedmem_addr);
		__uds_sharedmem_addr = NULL;
	}

	if(R_FAILED(ret) && __uds_connectionstatus_event)
	{
		svcCloseHandle(__uds_connectionstatus_event);
		__uds_connectionstatus_event = 0;
	}

	return ret;
}

Result udsCreateNetwork(udsNetworkStruct *network, void* passphrase, size_t passphrase_size, udsBindContext *context)
{
	Result ret=0;

	ret = udsipc_SetProbeResponseParam(0x00210080, 0);
	if(R_FAILED(ret))return ret;

	ret = udsipc_BeginHostingNetwork(network, passphrase, passphrase_size);
	if(R_FAILED(ret))return ret;

	ret = udsBind(context, UDS_BROADCAST_NETWORKNODEID);

	if(R_FAILED(ret))udsDestroyNetwork();

	return ret;
}

static Result udsipc_InitializeWithVersion(udsNodeInfo *nodeinfo, Handle sharedmem_handle, u32 sharedmem_size, Handle *eventhandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1B,12,2); // 0x1B0302
	cmdbuf[1]=sharedmem_size;
	memcpy(&cmdbuf[2], nodeinfo, sizeof(udsNodeInfo));
	cmdbuf[12] = 0x400;//version
	cmdbuf[13] = IPC_Desc_SharedHandles(1);
	cmdbuf[14] = sharedmem_handle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))
	{
		if(eventhandle)*eventhandle = cmdbuf[3];
	}

	return ret;
}

static Result udsipc_Shutdown(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x3,0,0); // 0x30000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

Result udsDestroyNetwork(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x8,0,0); // 0x80000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

Result udsBind(udsBindContext *bindcontext, u16 NetworkNodeID)
{
	u32 pos;

	memset(bindcontext, 0, sizeof(udsBindContext));

	for(pos=0; pos<UDS_MAXNODES; pos++)
	{
		if((bind_allocbitmask & BIT(pos)) == 0)break;
	}
	if(pos==UDS_MAXNODES)return -1;

	bind_allocbitmask |= BIT(pos);

	bindcontext->BindNodeID = (pos+1)<<1;

	return udsipc_Bind(bindcontext, 0x2e30, 0xf3, NetworkNodeID);
}

Result udsUnbind(udsBindContext *bindcontext)
{
	Result ret=0;

	if(bindcontext->event)
	{
		svcCloseHandle(bindcontext->event);
	}

	ret = udsipc_Unbind(bindcontext);

	bind_allocbitmask &= ~BIT((bindcontext->BindNodeID>>1) - 1);

	memset(bindcontext, 0, sizeof(udsBindContext));

	return ret;
}

static Result udsipc_Bind(udsBindContext *bindcontext, u32 input0, u8 input1, u16 NetworkNodeID)//input0 and input1 are unknown.
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x12,4,0); // 0x120100
	cmdbuf[1]=bindcontext->BindNodeID;
	cmdbuf[2]=input0;
	cmdbuf[3]=input1;
	cmdbuf[4]=NetworkNodeID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))
	{
		bindcontext->event = cmdbuf[3];
	}

	return ret;
}

static Result udsipc_Unbind(udsBindContext *bindcontext)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x13,1,0); // 0x130040
	cmdbuf[1]=bindcontext->BindNodeID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

Result udsSendTo(u16 dst_NetworkNodeID, u8 input8, u8 flags, void* buf, size_t size)
{
	u32* cmdbuf=getThreadCommandBuffer();

	u32 aligned_size = (size+0x3) & ~0x3;

	cmdbuf[0]=IPC_MakeHeader(0x17,6,2); // 0x170182
	cmdbuf[1]=0x1;//Unused
	cmdbuf[2]=dst_NetworkNodeID;
	cmdbuf[3]=input8;
	cmdbuf[4]=aligned_size>>2;
	cmdbuf[5]=size;
	cmdbuf[6]=flags;
	cmdbuf[7]=IPC_Desc_StaticBuffer(aligned_size, 5);
	cmdbuf[8]=(u32)buf;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

static Result udsipc_BeginHostingNetwork(udsNetworkStruct *network, void* passphrase, size_t passphrase_size)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1D,1,4); // 0x1D0044
	cmdbuf[1]=passphrase_size;
	cmdbuf[2]=IPC_Desc_StaticBuffer(sizeof(udsNetworkStruct), 1);
	cmdbuf[3]=(u32)network;
	cmdbuf[4]=IPC_Desc_StaticBuffer(passphrase_size, 0);
	cmdbuf[5]=(u32)network;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

static Result udsipc_SetProbeResponseParam(u32 oui, s8 data)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x21,2,0); // 0x210080
	cmdbuf[1]=oui;
	cmdbuf[2]=data;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

