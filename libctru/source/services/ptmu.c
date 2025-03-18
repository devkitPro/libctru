#include <stdlib.h>
#include <time.h>
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

Handle *ptmuGetSessionHandle(void)
{
	return &ptmuHandle;
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

Result PTMU_GetStepHistory(u32 hours, u16 *stepValue)
{
	Result ret=0;

	time_t raw;
	time(&raw);
	double msTime = difftime(raw, 0x386D4380); // 01.01.2000 00:00:00 UTC
	msTime = msTime * 1000.0f;

	s64 msiTime = (s64)msTime;
	u32 msiTimeLo, msiTimeHi;
	msiTimeLo = (u32)(msiTime & 0xFFFFFFFF);  // Low 32 Bit
	msiTimeHi = (u32)((msiTime >> 32) & 0xFFFFFFFF); // High 32 Bit

	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xB, 3, 2); // 0x00B00C2
	cmdbuf[1] = hours;
	cmdbuf[2] = msiTimeLo;
	cmdbuf[3] = msiTimeHi;
	cmdbuf[4] = IPC_Desc_Buffer(hours, IPC_BUFFER_W);
	cmdbuf[5] = (u32)stepValue;

	if(R_FAILED(ret = svcSendSyncRequest(ptmuHandle)))return ret;

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
