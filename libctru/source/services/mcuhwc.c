#include <3ds.h>
#include <3ds/services/mcuhwc.h>

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

Result mcuHwcReadRegister(u8 reg, void* data, u32 size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,2,2); // 0x10082
	cmdbuf[1] = reg;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer (size, IPC_BUFFER_W);
	cmdbuf[4] = (u32)data;

	if(R_FAILED(ret = svcSendSyncRequest(mcuHwcHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result mcuHwcWriteRegister(u8 reg, const void *data, u32 size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,2,2); // 0x20082
	cmdbuf[1] = reg;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer (size, IPC_BUFFER_R);
	cmdbuf[4] = (u32)data;

	if(R_FAILED(ret = svcSendSyncRequest(mcuHwcHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result mcuHwcGetBatteryVoltage(u8 *voltage)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4,0,0); // 0x40000

	if(R_FAILED(ret = svcSendSyncRequest(mcuHwcHandle)))return ret;

	*voltage = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result mcuHwcGetBatteryLevel(u8 *level)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5,0,0); // 0x50000

	if(R_FAILED(ret = svcSendSyncRequest(mcuHwcHandle)))return ret;

	*level = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result mcuHwcSetPowerLedState(powerLedState state)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6,2,0); // 0x60040
	cmdbuf[1] = state;
	
	if(R_FAILED(ret = svcSendSyncRequest(mcuHwcHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result mcuHwcSetWifiLedState(bool state)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x7,0,0); // 0x70000
	cmdbuf[1] = state;
	
	if(R_FAILED(ret = svcSendSyncRequest(mcuHwcHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result mcuHwcGetSoundSliderLevel(u8 *level)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB,0,0); // 0xB0000

	if(R_FAILED(ret = svcSendSyncRequest(mcuHwcHandle)))return ret;

	*level = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result mcuHwcGet3dSliderLevel(u8 *level)
{
	return mcuHwcReadRegister(8, &level, 1);
}