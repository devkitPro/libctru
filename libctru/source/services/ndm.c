#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/ndm.h>
#include <3ds/ipc.h>

Handle ndmuHandle;
static int ndmuRefCount;

Result ndmuInit(void)
{
	Result ret=0;

	if (AtomicPostIncrement(&ndmuRefCount)) return 0;

	ret = srvGetServiceHandle(&ndmuHandle, "ndm:u");
	if (R_FAILED(ret)) AtomicDecrement(&ndmuRefCount);

	return ret;
}

void ndmuExit(void)
{
	if (AtomicDecrement(&ndmuRefCount)) return;

	svcCloseHandle(ndmuHandle);
	ndmuHandle = 0;
}

Result NDMU_EnterExclusiveState(ndmExclusiveState state)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1,1,2); // 0x10042
	cmdbuf[1]=state;
	cmdbuf[2]=IPC_Desc_CurProcessHandle();

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result NDMU_LeaveExclusiveState(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x2,0,2); // 0x20002
	cmdbuf[1]=IPC_Desc_CurProcessHandle();

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result NDMU_GetExclusiveState(ndmExclusiveState *state)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x3,0,0); // 0x30000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	*state = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result NDMU_LockState(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x4,0,2); // 0x40002
	cmdbuf[1]=IPC_Desc_CurProcessHandle();

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result NDMU_UnlockState(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x5,0,2); // 0x50002
	cmdbuf[1]=IPC_Desc_CurProcessHandle();

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result NDMU_SuspendDaemons(ndmDaemonMask mask)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x6,1,0); // 0x60040
	cmdbuf[1]=mask;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result NDMU_ResumeDaemons(ndmDaemonMask mask)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x7,1,0); // 0x70040
	cmdbuf[1]=mask;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result NDMU_SuspendScheduler(u32 flag)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x8,1,0); // 0x80040
	cmdbuf[1]=flag;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result NDMU_ResumeScheduler(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x9,0,0); // 0x90000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result NDMU_GetCurrentState(ndmState *state)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xA,0,0); // 0xA0000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	*state = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result NDMU_QueryStatus(ndmDaemonStatus *status)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xD,1,0); // 0xD0000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	*status = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result NDMU_SetScanInterval(u32 interval)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x10,1,0); // 0x10040
	cmdbuf[1]=interval;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result NDMU_GetScanInterval(u32 *interval)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x11,0,0); // 0x110000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	*interval = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result NDMU_GetRetryInterval(u32 *interval)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x13,0,0); // 0x130000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	*interval = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result NDMU_ResetDaemons(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x15,0,0); // 0x150000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result NDMU_GetDefaultDaemons(ndmDaemonMask *mask)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x16,0,0); // 0x160000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	*mask = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result NDMU_ClearMacFilter(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x17,0,0); // 0x170000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(ndmuHandle)))return ret;

	return (Result)cmdbuf[1];
}
