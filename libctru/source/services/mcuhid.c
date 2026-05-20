#include <3ds/services/mcu_common.h>
#include <3ds/synchronization.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/ipc.h>
#include <3ds/srv.h>

#include <string.h>

static Handle mcuHidHandle;
static int mcuHidRefCount;

Result mcuHidInit(void)
{
	if (AtomicPostIncrement(&mcuHidRefCount)) return 0;
	Result res = srvGetServiceHandle(&mcuHidHandle, "mcu::HID");
	if (R_FAILED(res)) AtomicDecrement(&mcuHidRefCount);
	return res;
}

void mcuHidExit(void)
{
	if (AtomicDecrement(&mcuHidRefCount)) return;
	svcCloseHandle(mcuHidHandle);
}

Handle *mcuHidGetSessionHandle(void)
{
	return &mcuHidHandle;
}

Result MCUHID_SetAccelerometerEnabled(bool enable)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0001, 1, 0); // 0x00010040
	cmdbuf[1] = !!enable;

	Result res = svcSendSyncRequest(mcuHidHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUHID_GetAccelerometerEnabled(bool *out_enabled)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0002, 0, 0); // 0x00020000
	Result res = svcSendSyncRequest(mcuHidHandle);
	if (R_FAILED(res)) return res;

	*out_enabled = !!cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCUHID_StartAccelerometerManualRead(u8 hw_regid)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0003, 1, 0); // 0x00030040
	cmdbuf[1] = (u32)hw_regid;

	Result res = svcSendSyncRequest(mcuHidHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUHID_GetAccelerometerManualReadResult(u8 *out_data)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0004, 0, 0); // 0x00040000

	Result res = svcSendSyncRequest(mcuHidHandle);
	if (R_FAILED(res)) return res;

	*out_data = cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result MCUHID_PerformAccelerometerManualWrite(u8 hw_regid, u8 data)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0005, 2, 0); // 0x00050080
	cmdbuf[1] = (u32)hw_regid;
	cmdbuf[2] = (u32)data;

	Result res = svcSendSyncRequest(mcuHidHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUHID_ReadAccelerometerData(MCU_AccelerometerData *out_data)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0006, 0, 0); // 0x00060000

	Result res = svcSendSyncRequest(mcuHidHandle);
	if (R_FAILED(res)) return res;

	memcpy(out_data, &cmdbuf[2], sizeof(MCU_AccelerometerData));

	return (Result)cmdbuf[1];
}

Result MCUHID_Read3dSliderPosition(u8 *out_pos)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0007, 0, 0); // 0x00070000

	Result res = svcSendSyncRequest(mcuHidHandle);
	if (R_FAILED(res)) return res;

	*out_pos = cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result MCUHID_SetAccelerometerScale(MCU_AccelerometerScale scale)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0008, 1, 0); // 0x00080040
	cmdbuf[1] = scale;

	Result res = svcSendSyncRequest(mcuHidHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUHID_GetAccelerometerScale(MCU_AccelerometerScale *out_scale)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0009, 0, 0); // 0x00090000

	Result res = svcSendSyncRequest(mcuHidHandle);
	if (R_FAILED(res)) return res;

	*out_scale = (MCU_AccelerometerScale)(cmdbuf[2] & 0xFF);

	return (Result)cmdbuf[1];
}

Result MCUHID_SetAccelerometerInternalFilterEnabled(bool enabled)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000A, 1, 0); // 0x000A0040
	cmdbuf[1] = !!enabled;

	Result res = svcSendSyncRequest(mcuHidHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUHID_GetAccelerometerInternalFilterEnabled(bool *out_enabled)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000B, 0, 0); // 0x000B0000

	Result res = svcSendSyncRequest(mcuHidHandle);
	if (R_FAILED(res)) return res;

	*out_enabled = !!cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCUHID_GetInterruptEventHandle(Handle *out_handle)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000C, 0, 0); // 0x000C0000

	Result res = svcSendSyncRequest(mcuHidHandle);
	if (R_FAILED(res)) return res;

	*out_handle = cmdbuf[3];
	return (Result)cmdbuf[1];
}

Result MCUHID_GetReceivedEvents(u32 *out_irqs)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000D, 0, 0); // 0x000D0000

	Result res = svcSendSyncRequest(mcuHidHandle);
	if (R_FAILED(res)) return res;

	*out_irqs = cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result MCUHID_GetVolumeSliderLevel(u8 *level)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000E, 0, 0); // 0x000E0000

	Result res = svcSendSyncRequest(mcuHidHandle);
	if (R_FAILED(res)) return res;

	*level = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCUHID_SetAccelerometerIrqEnabled(bool enable)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000F, 1, 0); // 0x000F0040
	cmdbuf[1] = !!enable;

	Result res = svcSendSyncRequest(mcuHidHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}
