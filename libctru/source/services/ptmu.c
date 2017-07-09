#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/ptmu.h>
#include <3ds/ipc.h>

static Handle ptmuHandle;
static int ptmuRefCount;

Result ptmuInit(void)
{
	if (AtomicPostIncrement(&ptmuRefCount)) return 0;
	Result res = srvGetServiceHandle(&ptmuHandle, "ptm:u");
	if (R_FAILED(res)) AtomicDecrement(&ptmuRefCount);
	return res;
}

void ptmuExit(void)
{
	if (AtomicDecrement(&ptmuRefCount)) return;
	svcCloseHandle(ptmuHandle);
}

Result PTMU_GetShellState(u8 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6,0,0); // 0x60000

	if(R_FAILED(ret = svcSendSyncRequest(ptmuHandle)))return ret;

	*out = (u8)cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result PTMU_GetBatteryLevel(u8 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x7,0,0); // 0x70000

	if(R_FAILED(ret = svcSendSyncRequest(ptmuHandle)))return ret;

	*out = (u8)cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result PTMU_GetBatteryChargeState(u8 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x8,0,0); // 0x80000

	if(R_FAILED(ret = svcSendSyncRequest(ptmuHandle)))return ret;

	*out = (u8)cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result PTMU_GetPedometerState(u8 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9,0,0); // 0x90000

	if(R_FAILED(ret = svcSendSyncRequest(ptmuHandle)))return ret;

	*out = (u8)cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result PTMU_GetTotalStepCount(u32 *steps)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xC,0,0); // 0xC0000

	if(R_FAILED(ret = svcSendSyncRequest(ptmuHandle)))return ret;

	*steps = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result PTMU_GetAdapterState(bool *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x5,0,0); // 0x50000
	
	if(R_FAILED(ret = svcSendSyncRequest(ptmuHandle)))return ret;
	
	*out = cmdbuf[2] & 0xFF;
	
	return (Result)cmdbuf[1];
}
