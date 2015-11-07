#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/ac.h>
#include <3ds/ipc.h>

static Handle acHandle;
static int acRefCount;

Result acInit(void)
{
	Result ret;

	if (AtomicPostIncrement(&acRefCount)) return 0;

	ret = srvGetServiceHandle(&acHandle, "ac:u");
	if(R_FAILED(ret)) ret = srvGetServiceHandle(&acHandle, "ac:i");
	if(R_FAILED(ret)) AtomicDecrement(&acRefCount);

	return ret;
}

void acExit(void)
{
	if (AtomicDecrement(&acRefCount)) return;
	svcCloseHandle(acHandle);
}

// ptr=0x200-byte outbuf
Result ACU_CreateDefaultConfig(u32 *ptr)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 savedValue0 = staticbufs[0];
	u32 savedValue1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x1,0,0); // 0x00010000
	staticbufs[0] = IPC_Desc_StaticBuffer(0x200,0);
	staticbufs[1] = (u32)ptr;

	if(R_FAILED(ret = svcSendSyncRequest(acHandle)))return ret;

	staticbufs[0] = savedValue0;
	staticbufs[1] = savedValue1;

	return (Result)cmdbuf[1];
}

// Unknown what this cmd does at the time of writing. (ptr=0x200-byte inbuf/outbuf)
Result ACU_cmd26(u32 *ptr, u8 val)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 savedValue0 = staticbufs[0];
	u32 savedValue1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x26,1,2); // 0x00260042
	cmdbuf[1] = (u32)val;
	staticbufs[0] = IPC_Desc_StaticBuffer(0x200,0);
	staticbufs[1] = (u32)ptr;
	cmdbuf[2] = IPC_Desc_StaticBuffer(0x200,0);
	cmdbuf[3] = (u32)ptr;

	if(R_FAILED(ret = svcSendSyncRequest(acHandle)))return ret;

	staticbufs[0] = savedValue0;
	staticbufs[1] = savedValue1;

	return (Result)cmdbuf[1];
}

Result ACU_GetWifiStatus(u32 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xD,0,0); // 0x000D0000

	if(R_FAILED(ret = svcSendSyncRequest(acHandle)))return ret;

	*out = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result ACU_WaitInternetConnection(void)
{
	Result ret=0;
	u32 outval=0;

	if(R_FAILED(ret = acInit()))return ret;

	while(1)
	{
		ret = ACU_GetWifiStatus(&outval);
		if(R_SUCCEEDED(ret) && outval!=0)break;
	}

	acExit();

	return ret;
}
