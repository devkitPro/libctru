#include <3ds.h>
#include <3ds/services/mcuHwc.h>

static Handle mcuHWCHandle;
static int mcuHWCRefCount;

Result mcuHWCInit(void)
{
	if (AtomicPostIncrement(&mcuHWCRefCount)) return 0;
	Result res = srvGetServiceHandle(&mcuHWCHandle, "mcu::HWC");
	if (R_FAILED(res)) AtomicDecrement(&mcuHWCRefCount);
	return res;
}

void mcuHWCExit(void)
{
	if (AtomicDecrement(&mcuHWCRefCount)) return;
	svcCloseHandle(mcuHWCHandle);
}

Result mcuReadRegister(u8 reg, void* data, u32 size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,2,2); // 0x10082
	cmdbuf[1] = reg;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer (size, IPC_BUFFER_W);
	cmdbuf[4] = (u32)data;

	if(R_FAILED(ret = svcSendSyncRequest(mcuHWCHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result mcuWriteRegister(u8 reg, const void *data, u32 size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,2,2); // 0x20082
	cmdbuf[1] = reg;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer (size, IPC_BUFFER_R);
	cmdbuf[4] = (u32)data;

	if(R_FAILED(ret = svcSendSyncRequest(mcuHWCHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result mcuGetBatteryVoltage(u8 *voltage)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4,0,0); // 0x40000

	if(R_FAILED(ret = svcSendSyncRequest(mcuHWCHandle)))return ret;

	*voltage = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result mcuGetBatteryLevel(u8 *level)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5,0,0); // 0x50000

	if(R_FAILED(ret = svcSendSyncRequest(mcuHWCHandle)))return ret;

	*level = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result mcuGetSoundSliderLevel(u8 *level)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB,0,0); // 0xB0000

	if(R_FAILED(ret = svcSendSyncRequest(mcuHWCHandle)))return ret;

	*level = cmdbuf[2];

	return (Result)cmdbuf[1];
}