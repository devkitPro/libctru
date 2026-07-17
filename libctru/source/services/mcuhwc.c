#include <3ds/synchronization.h>
#include <3ds/services/mcuhwc.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/ipc.h>
#include <3ds/srv.h>

#include <string.h>

static Handle mcuHwcHandle;
static int mcuHwcRefCount;

Result mcuHwcInit(void)
{
	if (AtomicPostIncrement(&mcuHwcRefCount)) return 0;
	Result res = srvGetServiceHandle(&mcuHwcHandle, "mcu::HWC");
	if (R_FAILED(res)) AtomicDecrement(&mcuHwcRefCount);
	return res;
}

void mcuHwcExit(void)
{
	if (AtomicDecrement(&mcuHwcRefCount)) return;
	svcCloseHandle(mcuHwcHandle);
}

Handle *mcuHwcGetSessionHandle(void)
{
	return &mcuHwcHandle;
}

Result MCUHWC_ReadRegister(u8 reg, void *data, u32 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0001, 2, 2); // 0x00010082
	cmdbuf[1] = reg;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer (size, IPC_BUFFER_W);
	cmdbuf[4] = (u32)data;

	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUHWC_WriteRegister(u8 reg, const void *data, u32 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0002, 2, 2); // 0x00020082
	cmdbuf[1] = reg;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer (size, IPC_BUFFER_R);
	cmdbuf[4] = (u32)data;

	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUHWC_ReadInfoRegister(void *data, u8 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0003, 1, 2); // 0x00030042
	cmdbuf[1] = (u32)size;
	cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[3] = (u32)data;
	
	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUHWC_GetBatteryVoltage(u8 *voltage)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0004, 0, 0); // 0x00040000

	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;

	*voltage = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCUHWC_GetBatteryLevel(u8 *level)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0005, 0, 0); // 0x00050000

	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;

	*level = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCUHWC_SetPowerLedState(MCU_PowerLedState state)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0006, 1, 0); // 0x00060040
	cmdbuf[1] = state;

	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUHWC_SetWifiLedState(bool state)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0007, 1, 0); // 0x00070040
	cmdbuf[1] = !!state;

	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUHWC_SetCameraLedState(bool state)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0008, 1, 0); // 0x00080040
	cmdbuf[1] = !!state;

	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUHWC_Set3dLedState(bool state)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0009, 1, 0); // 0x00090040
	cmdbuf[1] = !!state;

	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUHWC_SetInfoLedPattern(const MCU_InfoLedPattern *pattern)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000A, 25, 0); // 0x000A0640
	memcpy(&cmdbuf[1], pattern, sizeof(MCU_InfoLedPattern));

	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUHWC_GetVolumeSliderLevel(u8 *level)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000B, 0, 0); // 0x000B0000

	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;

	*level = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCUHWC_SetTopLcdFlicker(u8 flicker)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x000C, 1, 0); // 0x000C0040
	cmdbuf[1] = (u32)flicker;
	
	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUHWC_SetBottomLcdFlicker(u8 flicker)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x000D, 1, 0); // 0x000D0040
	cmdbuf[1] = (u32)flicker;
	
	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUHWC_GetBatteryPcbTemperature(s8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x000E, 0, 0); // 0x000E0000
	
	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;
	
	*out_value = (s8)(cmdbuf[2] & 0xFF);
	
	return (Result)cmdbuf[1];
}

Result MCUHWC_GetRtcTime(MCU_RtcTime *out_time)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x000F, 0, 0); // 0x000F0000
	
	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;
	
	memcpy(out_time, &cmdbuf[2], sizeof(MCU_RtcTime));
	
	return (Result)cmdbuf[1];
}

Result MCUHWC_GetFwVerHigh(u8 *out)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0010, 0, 0); // 0x00100000

	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;

	*out = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCUHWC_GetFwVerLow(u8 *out)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0011, 0, 0); // 0x00110000

	Result res = svcSendSyncRequest(mcuHwcHandle);
	if (R_FAILED(res)) return res;

	*out = cmdbuf[2];

	return (Result)cmdbuf[1];
}
