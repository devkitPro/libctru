#include <3ds/services/mcu_common.h>
#include <3ds/synchronization.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/ipc.h>
#include <3ds/srv.h>

static Handle mcuNwmHandle;
static int mcuNwmRefCount;

Result mcuNwmInit(void)
{
	if (AtomicPostIncrement(&mcuNwmRefCount)) return 0;
	Result res = srvGetServiceHandle(&mcuNwmHandle, "mcu::NWM");
	if (R_FAILED(res)) AtomicDecrement(&mcuNwmRefCount);
	return res;
}

void mcuNwmExit(void)
{
	if (AtomicDecrement(&mcuNwmRefCount)) return;
	svcCloseHandle(mcuNwmHandle);
}

Handle *mcuNwmGetSessionHandle(void)
{
	return &mcuNwmHandle;
}

Result MCUNWM_SetWifiLedState(bool state)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0001, 1, 0); // 0x00010040
	cmdbuf[1] = !!state;

	Result res = svcSendSyncRequest(mcuNwmHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUNWM_GetWifiLedState(bool *out_state)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0002, 0, 0); // 0x00020000
	
	Result res = svcSendSyncRequest(mcuNwmHandle);
	if (R_FAILED(res)) return res;
	
	*out_state = !!cmdbuf[2];
	
	return (Result)cmdbuf[1];
}

Result MCUNWM_SetWifiMode(MCU_WifiMode mode)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0003, 1, 0); // 0x00030040
	cmdbuf[1] = mode;
	
	Result res = svcSendSyncRequest(mcuNwmHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUNWM_GetWifiMode(MCU_WifiMode *out_mode)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0004, 0, 0); // 0x00040000
	
	Result res = svcSendSyncRequest(mcuNwmHandle);
	if (R_FAILED(res)) return res;
	
	*out_mode = (MCU_WifiMode)(cmdbuf[2] & 0xFF);
	
	return (Result)cmdbuf[1];
}

Result MCUNWM_SetWifiEnabled(bool enabled)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0005, 1, 0); // 0x00050040
	cmdbuf[1] = !!enabled;
	
	Result res = svcSendSyncRequest(mcuNwmHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUNWM_GetWifiEnabled(bool *out_enabled)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0006, 0, 0); // 0x00060000
	
	Result res = svcSendSyncRequest(mcuNwmHandle);
	if (R_FAILED(res)) return res;
	
	*out_enabled = !!cmdbuf[2];
	
	return (Result)cmdbuf[1];
}

Result MCUNWM_SetWirelessDisabledFlag(bool value)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0007, 1, 0); // 0x00070040
	cmdbuf[1] = !!value;
	
	Result res = svcSendSyncRequest(mcuNwmHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUNWM_GetWirelessDisabledFlag(bool *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0008, 0, 0); // 0x00080000
	
	Result res = svcSendSyncRequest(mcuNwmHandle);
	if (R_FAILED(res)) return res;
	
	*out_value = !!cmdbuf[2];
	
	return (Result)cmdbuf[1];
}