#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
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

static Result uds_Initialize(u32 sharedmem_size, const uint8_t *username);
static Result udsipc_InitializeWithVersion(udsNodeInfo *nodeinfo, Handle sharedmem_handle, u32 sharedmem_size, Handle *eventhandle);
static Result udsipc_Shutdown(void);

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

