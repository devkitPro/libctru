#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/ptmsysm.h>
#include <3ds/ipc.h>

static Handle ptmSysmHandle;
static int ptmSysmRefCount;

Result ptmSysmInit(void)
{
	if (AtomicPostIncrement(&ptmSysmRefCount)) return 0;
	Result res = srvGetServiceHandle(&ptmSysmHandle, "ptm:sysm");
	if (R_FAILED(res)) AtomicDecrement(&ptmSysmRefCount);
	return res;
}

void ptmSysmExit(void)
{
	if (AtomicDecrement(&ptmSysmRefCount)) return;
	svcCloseHandle(ptmSysmHandle);
}

Handle *ptmSysmGetSessionHandle(void)
{
	return &ptmSysmHandle;
}

Result PTMSYSM_RequestSleep(void)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x0406,0,0); // 0x04060000

	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PTMSYSM_ReplyToSleepQuery(bool deny)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x0402,1,2); // 0x04020042
	cmdbuf[1] = (u32)deny;
	cmdbuf[2] = IPC_Desc_CurProcessId();
	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PTMSYSM_NotifySleepPreparationComplete(s32 ackValue)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x0403,1,2); // 0x04030042
	cmdbuf[1] = (u32)ackValue;
	cmdbuf[2] = IPC_Desc_CurProcessId();
	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PTMSYSM_SetWakeEvents(const PtmSleepConfig *sleepConfig)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x0404,4,2); // 0x04040102
	memcpy(&cmdbuf[1], sleepConfig, sizeof(PtmSleepConfig));
	cmdbuf[5] = IPC_Desc_CurProcessId();
	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PTMSYSM_GetWakeReason(PtmSleepConfig *outSleepConfig)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x0405,0,0); // 0x04050000
	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;
	memcpy(outSleepConfig, &cmdbuf[2], sizeof(PtmSleepConfig));

	return (Result)cmdbuf[1];
}

Result PTMSYSM_Awaken(void)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x0408,0,0); // 0x04080000

	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PTMSYSM_CheckNew3DS(bool *out)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x040A,0,0); // 0x040A0000

	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;
	*out = (bool)(cmdbuf[2] & 1); // if cmdbuf[1] is != 0 then this is uninitialized (this is fine)

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

Result PTMSYSM_ShutdownAsync(u64 timeout)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x407,3,0); // 0x040700C0
	cmdbuf[1] = 0;
	cmdbuf[2] = timeout & 0xffffffff;
	cmdbuf[3] = (timeout >> 32) & 0xffffffff;

	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PTMSYSM_RebootAsync(u64 timeout)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x409,2,0); // 0x04090080
	cmdbuf[1] = timeout & 0xffffffff;
	cmdbuf[2] = (timeout >> 32) & 0xffffffff;

	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PTMSYSM_SetUserTime(s64 msY2k)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x80C,2,0); // 0x080C0080
	memcpy(&cmdbuf[1], &msY2k, 8);

	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PTMSYSM_InvalidateSystemTime(void)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x080D,0,0); // 0x080D0000

	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PTMSYSM_GetRtcTime(s64 *outMsY2k)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x0816,0,0); // 0x08160000
	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;
	memcpy(outMsY2k, &cmdbuf[2], 8);

	return (Result)cmdbuf[1];
}

Result PTMSYSM_SetRtcTime(s64 msY2k)
{
	Result ret;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x0817,2,0); // 0x08170080
	memcpy(&cmdbuf[1], &msY2k, 8);
	if(R_FAILED(ret = svcSendSyncRequest(ptmSysmHandle)))return ret;

	return (Result)cmdbuf[1];
}
