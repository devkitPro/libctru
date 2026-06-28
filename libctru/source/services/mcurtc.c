#include <3ds/services/mcu_common.h>
#include <3ds/synchronization.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/ipc.h>
#include <3ds/srv.h>

#include <string.h>

static Handle mcuRtcHandle;
static int mcuRtcRefCount;

Result mcuRtcInit(void)
{
	if (AtomicPostIncrement(&mcuRtcRefCount)) return 0;
	Result res = srvGetServiceHandle(&mcuRtcHandle, "mcu::RTC");
	if (R_FAILED(res)) AtomicDecrement(&mcuRtcRefCount);
	return res;
}

void mcuRtcExit(void)
{
	if (AtomicDecrement(&mcuRtcRefCount)) return;
	svcCloseHandle(mcuRtcHandle);
}

Handle *mcuRtcGetSessionHandle(void)
{
	return &mcuRtcHandle;
}

Result MCURTC_SetRtcTime(const MCU_RtcTime *time)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0001, 2, 0); // 0x00010080
	memcpy(&cmdbuf[1], time, sizeof(MCU_RtcTime));

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTime(MCU_RtcTime *out_time, s64 *out_tick)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0002, 0, 0); // 0x00020000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	memcpy(out_time, &cmdbuf[2], sizeof(MCU_RtcTime));
	*out_tick = *((s64 *)&cmdbuf[4]);

	return (Result)cmdbuf[1];
}

Result MCURTC_SetRtcTimeSeconds(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0003, 1, 0); // 0x00030040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTimeSeconds(u8 *out_value, s64 *out_tick)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0004, 0, 0); // 0x00040000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = cmdbuf[2] & 0xFF;
	*out_tick = *((s64 *)&cmdbuf[3]);

	return (Result)cmdbuf[1];
}

Result MCURTC_SetRtcTimeMinute(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0005, 1, 0); // 0x00050040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTimeMinute(u8 *out_value, s64 *out_tick)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0006, 0, 0); // 0x00060000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = cmdbuf[2] & 0xFF;
	*out_tick = *((s64 *)&cmdbuf[3]);

	return (Result)cmdbuf[1];
}

Result MCURTC_SetRtcTimeHour(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0007, 1, 0); // 0x00070040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTimeHour(u8 *out_value, s64 *out_tick)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0008, 0, 0); // 0x00080000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = cmdbuf[2] & 0xFF;
	*out_tick = *((s64 *)&cmdbuf[3]);

	return (Result)cmdbuf[1];
}

Result MCURTC_SetRtcTimeWeekday(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0009, 1, 0); // 0x00090040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTimeWeekday(u8 *out_value, s64 *out_tick)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000A, 0, 0); // 0x000A0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = cmdbuf[2] & 0xFF;
	*out_tick = *((s64 *)&cmdbuf[3]);

	return (Result)cmdbuf[1];
}

Result MCURTC_SetRtcTimeDay(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000B, 1, 0); // 0x000B0040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTimeDay(u8 *out_value, s64 *out_tick)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000C, 0, 0); // 0x000C0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = cmdbuf[2] & 0xFF;
	*out_tick = *((s64 *)&cmdbuf[3]);

	return (Result)cmdbuf[1];
}

Result MCURTC_SetRtcTimeMonth(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000D, 1, 0); // 0x000D0040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTimeMonth(u8 *out_value, s64 *out_tick)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000E, 0, 0); // 0x000E0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = cmdbuf[2] & 0xFF;
	*out_tick = *((s64 *)&cmdbuf[3]);

	return (Result)cmdbuf[1];
}

Result MCURTC_SetRtcTimeYear(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000F, 1, 0); // 0x000F0040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTimeYear(u8 *out_value, s64 *out_tick)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0010, 0, 0); // 0x00100000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = cmdbuf[2] & 0xFF;
	*out_tick = *((s64 *)&cmdbuf[3]);

	return (Result)cmdbuf[1];
}

Result MCURTC_SetRtcTimeCorrection(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0011, 1, 0); // 0x00110040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcTimeCorrection(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0012, 0, 0); // 0x00120000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result MCURTC_SetRtcAlarmTime(const MCU_RtcAlarmTime *time)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0013, 2, 0); // 0x00130080
	memcpy(&cmdbuf[1], time, sizeof(MCU_RtcAlarmTime));

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcAlarmTime(MCU_RtcAlarmTime *out_time)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0014, 0, 0); // 0x00140000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	memcpy(out_time, &cmdbuf[2], sizeof(MCU_RtcAlarmTime));

	return (Result)cmdbuf[1];
}

Result MCURTC_SetRtcAlarmTimeMinute(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0015, 1, 0); // 0x00150040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcAlarmTimeMinute(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0016, 0, 0); // 0x00160000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result MCURTC_SetRtcAlarmTimeHour(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0017, 1, 0); // 0x00170040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcAlarmTimeHour(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0018, 0, 0); // 0x00180000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result MCURTC_SetRtcAlarmTimeDay(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0019, 1, 0); // 0x00190040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcAlarmTimeDay(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x001A, 0, 0); // 0x001A0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result MCURTC_SetRtcAlarmTimeMonth(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x001B, 1, 0); // 0x001B0040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcAlarmTimeMonth(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x001C, 0, 0); // 0x001C0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result MCURTC_SetRtcAlarmTimeYear(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x001D, 1, 0); // 0x001D0040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetRtcAlarmTimeYear(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x001E, 0, 0); // 0x001E0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result MCURTC_SetPedometerEnabled(bool enabled)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x001F, 1, 0); // 0x001F0040
	cmdbuf[1] = !!enabled;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetPedometerEnabled(bool *out_enabled)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0020, 0, 0); // 0x00200000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_enabled = !!cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCURTC_ReadPedometerStepCount(u32 *out_num_steps)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0021, 0, 0); // 0x00210000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_num_steps = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCURTC_ReadPedometerStepData(MCU_PedometerStepData *out_data)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0022, 1, 2); // 0x00220042
	cmdbuf[1] = sizeof(MCU_PedometerStepData);
	cmdbuf[2] = IPC_Desc_Buffer(sizeof(MCU_PedometerStepData), IPC_BUFFER_W);
	cmdbuf[3] = (u32)out_data;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_ClearStepData()
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0023, 0, 0); // 0x00230000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetInterruptEventHandle(Handle *out_handle)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0024, 0, 0); // 0x00240000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_handle = cmdbuf[3];
	return (Result)cmdbuf[1];
}

Result MCURTC_GetReceivedInterrupts(u32 *out_irqs)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0025, 0, 0); // 0x00250000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_irqs = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCURTC_CheckRtcTimeLost(bool *out_lost)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0026, 0, 0); // 0x00260000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_lost = !!cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCURTC_ClearRtcTimeLost()
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0027, 0, 0); // 0x00270000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_CheckWatchdogResetOccurred(bool *out_occurred)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0028, 0, 0); // 0x00280000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_occurred = !!cmdbuf[2];

	return (Result)cmdbuf[1];
}

/**
  * @brief Clears the "watchdog reset occurred" flag.
  */
Result MCURTC_ClearWatchdogResetOccurred()
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0029, 0, 0); // 0x00290000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetShellState(bool *out_open)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x002A, 0, 0); // 0x002A0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_open = !!cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCURTC_GetAdapterState(bool *out_connected)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x002B, 0, 0); // 0x002B0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_connected = !!cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCURTC_GetChargingState(bool *out_connected)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x002C, 0, 0); // 0x002C0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_connected = !!cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCURTC_GetBatteryLevel(u8 *out_level)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x002D, 0, 0); // 0x002D0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_level = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCURTC_SetPowerLedState(MCU_PowerLedState state)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x002E, 1, 0); // 0x002E0040
	cmdbuf[1] = state;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	return (Result)cmdbuf[1];
}

Result MCURTC_GetPowerLedState(MCU_PowerLedState *out_state)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x002F, 0, 0); // 0x002F0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_state = (MCU_PowerLedState)(cmdbuf[2] & 0xFF);

	return (Result)cmdbuf[1];
}

Result MCURTC_SetLedBrightness(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0030, 1, 0); // 0x00300040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetLedBrightness(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0031, 0, 0); // 0x00310000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = (u8)(cmdbuf[2] & 0xFF);

	return (Result)cmdbuf[1];
}

Result MCURTC_Poweroff()
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0032, 0, 0); // 0x00320000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_Reboot()
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0033, 0, 0); // 0x00330000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_Reset()
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0034, 0, 0); // 0x00340000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_SignalEnterSleepMode()
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0035, 0, 0); // 0x00350000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_SetForceShutdownDelay(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0036, 1, 0); // 0x00360040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetForceShutdownDelay(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0037, 0, 0); // 0x00370000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = (u8)(cmdbuf[2] & 0xFF);

	return (Result)cmdbuf[1];
}

Result MCURTC_ReadInfoRegister(void *data, u8 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0038, 1, 2); // 0x00380042
	cmdbuf[1] = (u32)size;
	cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[3] = (u32)data;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_WriteStorageArea(u8 offset, const void *buf, u8 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0039, 2, 2); // 0x00390082
	cmdbuf[1] = offset;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[4] = (u32)buf;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_ReadStorageArea(u8 offset, void *buf, u8 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x003A, 2, 2); // 0x003A0082
	cmdbuf[1] = offset;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[4] = (u32)buf;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_SetInfoLedPattern(const MCU_InfoLedPattern *pattern)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x003B, 25, 0); // 0x003B0640
	memcpy(&cmdbuf[1], pattern, sizeof(MCU_InfoLedPattern));

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_SetInfoLedAnimation(const MCU_InfoLedAnimation *animation)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x003C, 1, 0); // 0x003C0040
	memcpy(&cmdbuf[1], animation, sizeof(MCU_InfoLedAnimation));

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetInfoLedCycleStatus(bool *out_new_cycle_started)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x003D, 0, 0); // 0x003D0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_new_cycle_started = !!cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCURTC_SetPedometerWrapTimeMinute(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x003E, 1, 0); // 0x003E0040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetPedometerWrapTimeMinute(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x003F, 0, 0); // 0x003F0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = (u8)(cmdbuf[2] & 0xFF);

	return (Result)cmdbuf[1];
}

Result MCURTC_SetPedometerWrapTimeSecond(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0040, 1, 0); // 0x00400040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetPedometerWrapTimeSecond(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0041, 0, 0); // 0x00410000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = (u8)(cmdbuf[2] & 0xFF);

	return (Result)cmdbuf[1];
}

Result MCURTC_SetPowerLedBlinkPattern(u32 pattern)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0042, 1, 0); // 0x00420040
	cmdbuf[1] = pattern;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_SetTopLcdFlicker(u8 flicker)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0043, 1, 0); // 0x00430040
	cmdbuf[1] = (u32)flicker;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetTopLcdFlicker(u8 *out_flicker)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0044, 0, 0); // 0x00440000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_flicker = cmdbuf[2] & 0xFF;
	return (Result)cmdbuf[1];
}

Result MCURTC_SetBottomLcdFlicker(u8 flicker)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0045, 1, 0); // 0x00450040
	cmdbuf[1] = (u32)flicker;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetBottomLcdFlicker(u8 *out_flicker)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0046, 0, 0); // 0x00460000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_flicker = cmdbuf[2] & 0xFF;
	return (Result)cmdbuf[1];
}

Result MCURTC_SetVolumeSliderBounds(u8 min, u8 max)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0047, 2, 0); // 0x00470080
	cmdbuf[1] = min;
	cmdbuf[2] = max;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetVolumeSliderBounds(u8 *out_min, u8 *out_max)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0048, 0, 0); // 0x00480000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_min = (u8)(cmdbuf[2] & 0xFF);
	*out_max = (u8)(cmdbuf[3] & 0xFF);

	return (Result)cmdbuf[1];
}

Result MCURTC_SetInterruptMask(u32 mask)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0049, 1, 0); // 0x00490040
	cmdbuf[1] = mask;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetInterruptMask(u32 *out_mask)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x004A, 0, 0); // 0x004A0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_mask = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCURTC_LeaveExclusiveInterruptMode()
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x004B, 0, 0); // 0x004B0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_EnterExclusiveInterruptMode()
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x004C, 0, 0); // 0x004C0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_ReadInterrupts(u32 *out_interrupts)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x004D, 0, 0); // 0x004D0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_interrupts = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCURTC_TriggerInterrupts(u32 interrupts)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x004E, 1, 0); // 0x004E0040
	cmdbuf[1] = interrupts;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_SetMcuFirmUpdatedFlag(bool value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x004F, 1, 0); // 0x004F0040
	cmdbuf[1] = !!value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetMcuFirmUpdatedFlag(bool *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0050, 0, 0); // 0x00500000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = !!cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCURTC_SetSoftwareClosedFlag(bool value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0051, 1, 0); // 0x00510040
	cmdbuf[1] = !!value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetSoftwareClosedFlag(bool *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0052, 0, 0); // 0x00520000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = !!cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCURTC_SetLgyLcdSettings(MCU_LcdSettings settings)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0053, 1, 0); // 0x00530040
	cmdbuf[1] = *((u8 *)&settings);

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetLgyLcdSettings(MCU_LcdSettings *out_settings)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0054, 0, 0); // 0x00540000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_settings = *((MCU_LcdSettings *)&cmdbuf[2]);

	return (Result)cmdbuf[1];
}

Result MCURTC_SetLgyNativeResolutionFlag(bool value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0055, 1, 0); // 0x00550040
	cmdbuf[1] = !!value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetLgyNativeResolutionFlag(bool *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0056, 0, 0); // 0x00560000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = !!cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCURTC_SetLocalFriendCodeCounter(u16 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0057, 1, 0); // 0x00570040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetLocalFriendCodeCounter(u16 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0058, 0, 0); // 0x00580000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = (u16)(cmdbuf[2] & 0xFFFF);

	return (Result)cmdbuf[1];
}

Result MCURTC_SetLegacyJumpProhibitedFlag(bool value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0059, 1, 0); // 0x00590040
	cmdbuf[1] = !!value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetLegacyJumpProhibitedFlag(bool *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x005A, 0, 0); // 0x005A0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = !!cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCURTC_SetUuidClockSequence(u16 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x005B, 1, 0); // 0x005B0040
	cmdbuf[1] = value;

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCURTC_GetUuidClockSequence(u16 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x005C, 0, 0); // 0x005C0000

	Result res = svcSendSyncRequest(mcuRtcHandle);
	if (R_FAILED(res)) return res;

	*out_value = (u16)(cmdbuf[2] & 0xFFFF);

	return (Result)cmdbuf[1];
}
