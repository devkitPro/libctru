#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/ptm.h>
#include <3ds/ipc.h>

static Handle ptmHandle, ptmSysmHandle;
static int ptmRefCount, ptmSysmRefCount;

Result ptmInit(void)
{
	if (AtomicPostIncrement(&ptmRefCount)) return 0;
	Result res = srvGetServiceHandle(&ptmHandle, "ptm:u");
	if (R_FAILED(res)) AtomicDecrement(&ptmRefCount);
	return res;
}

void ptmExit(void)
{
	if (AtomicDecrement(&ptmRefCount)) return;
	svcCloseHandle(ptmHandle);
}

Result ptmSysmInit(void)
{
	if (AtomicPostIncrement(&ptmSysmRefCount)) return 0;
	Result res = srvGetServiceHandle(&ptmSysmHandle, "ptm:sysm");
	if (R_FAILED(res)) AtomicDecrement(&ptmSysmHandle);
	return res;
}

void ptmSysmExit(void)
{
	if (AtomicDecrement(&ptmSysmHandle)) return;
	svcCloseHandle(ptmSysmHandle);
}

Result PTMU_GetShellState(u8 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6,0,0); // 0x60000

	if(R_FAILED(ret = svcSendSyncRequest(ptmHandle)))return ret;

	*out = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result PTMU_GetBatteryLevel(u8 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x7,0,0); // 0x70000

	if(R_FAILED(ret = svcSendSyncRequest(ptmHandle)))return ret;

	*out = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result PTMU_GetBatteryChargeState(u8 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x8,0,0); // 0x80000

	if(R_FAILED(ret = svcSendSyncRequest(ptmHandle)))return ret;

	*out = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result PTMU_GetPedometerState(u8 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9,0,0); // 0x90000

	if(R_FAILED(ret = svcSendSyncRequest(ptmHandle)))return ret;

	*out = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result PTMU_GetTotalStepCount(u32 *steps)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xC,0,0); // 0xC0000

	if(R_FAILED(ret = svcSendSyncRequest(ptmHandle)))return ret;

	*steps = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result PTMSYSM_ConfigureNew3DSCPU(u8 value)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x818,1,0); // 0x08180040
	cmdbuf[1] = value;

	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

	return (Result)cmdbuf[1];
}
