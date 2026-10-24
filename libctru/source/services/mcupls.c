#include <3ds/services/mcu_common.h>
#include <3ds/synchronization.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/ipc.h>
#include <3ds/srv.h>

#include <string.h>

static Handle mcuPlsHandle;
static int mcuPlsRefCount;

Result mcuPlsInit(void)
{
	if (AtomicPostIncrement(&mcuPlsRefCount)) return 0;
	Result res = srvGetServiceHandle(&mcuPlsHandle, "mcu::PLS");
	if (R_FAILED(res)) AtomicDecrement(&mcuPlsRefCount);
	return res;
}

void mcuPlsExit(void)
{
	if (AtomicDecrement(&mcuPlsRefCount)) return;
	svcCloseHandle(mcuPlsHandle);
}

Handle *mcuPlsGetSessionHandle(void)
{
	return &mcuPlsHandle;
}

Result MCURTC_GetRtcTime(MCU_RtcTime *out_time)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0001, 0, 0); // 0x00010000
	
	Result res = svcSendSyncRequest(mcuPlsHandle);
	if (R_FAILED(res)) return res;
	
	memcpy(out_time, &cmdbuf[2], sizeof(MCU_RtcTime));
	
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTimeSeconds(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0002, 0, 0); // 0x00020000
	
	Result res = svcSendSyncRequest(mcuPlsHandle);
	if (R_FAILED(res)) return res;
	
	*out_value = cmdbuf[2] & 0xFF;
	
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTimeMinute(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0003, 0, 0); // 0x00030000
	
	Result res = svcSendSyncRequest(mcuPlsHandle);
	if (R_FAILED(res)) return res;
	
	*out_value = cmdbuf[2] & 0xFF;
	
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTimeHour(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0004, 0, 0); // 0x00040000
	
	Result res = svcSendSyncRequest(mcuPlsHandle);
	if (R_FAILED(res)) return res;
	
	*out_value = cmdbuf[2] & 0xFF;
	
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTimeWeekday(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0005, 0, 0); // 0x00050000
	
	Result res = svcSendSyncRequest(mcuPlsHandle);
	if (R_FAILED(res)) return res;
	
	*out_value = cmdbuf[2] & 0xFF;
	
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTimeDay(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0006, 0, 0); // 0x00060000
	
	Result res = svcSendSyncRequest(mcuPlsHandle);
	if (R_FAILED(res)) return res;
	
	*out_value = cmdbuf[2] & 0xFF;
	
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTimeMonth(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0007, 0, 0); // 0x00070000
	
	Result res = svcSendSyncRequest(mcuPlsHandle);
	if (R_FAILED(res)) return res;
	
	*out_value = cmdbuf[2] & 0xFF;
	
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTimeYear(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0008, 0, 0); // 0x00080000
	
	Result res = svcSendSyncRequest(mcuPlsHandle);
	if (R_FAILED(res)) return res;
	
	*out_value = cmdbuf[2] & 0xFF;
	
	return (Result)cmdbuf[1];
}

Result MCUPLS_GetTickCounter(u16 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0009, 0, 0); // 0x00090000
	
	Result res = svcSendSyncRequest(mcuPlsHandle);
	if (R_FAILED(res)) return res;
	
	*out_value = (u16)(cmdbuf[2] & 0xFFFF);
	
	return (Result)cmdbuf[1];
}
