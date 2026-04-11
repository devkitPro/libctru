#include <3ds/synchronization.h>
#include <3ds/services/mcugpu.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/ipc.h>
#include <3ds/srv.h>

static Handle mcuGpuHandle;
static int mcuGpuRefCount;

Result mcuGpuInit(void)
{
	if (AtomicPostIncrement(&mcuGpuRefCount)) return 0;
	Result res = srvGetServiceHandle(&mcuGpuHandle, "mcu::GPU");
	if (R_FAILED(res)) AtomicDecrement(&mcuGpuRefCount);
	return res;
}

void mcuGpuExit(void)
{
	if (AtomicDecrement(&mcuGpuRefCount)) return;
	svcCloseHandle(mcuGpuHandle);
}

Handle *mcuGpuGetSessionHandle(void)
{
	return &mcuGpuHandle;
}

Result MCUGPU_GetBacklightPower(bool *out_top_on, bool *out_bot_on)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0001, 0, 0); // 0x00010000
	
	Result res = svcSendSyncRequest(mcuGpuHandle);
	
	if (R_FAILED(res)) return res;
	
	*out_top_on = !!cmdbuf[2];
	*out_bot_on = !!cmdbuf[3];
	return (Result)cmdbuf[1];
}

Result MCUGPU_SetBacklightPower(bool top_on, bool bot_on)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0002, 2, 0); // 0x00020080
	cmdbuf[1] = !!top_on;
	cmdbuf[2] = !!bot_on;
	
	Result res = svcSendSyncRequest(mcuGpuHandle);
	
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUGPU_GetLcdPower(bool *out_on)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0003, 0, 0); // 0x00030000
	
	Result res = svcSendSyncRequest(mcuGpuHandle);
	
	if (R_FAILED(res)) return res;
	
	*out_on = !!cmdbuf[2];
	return (Result)cmdbuf[1];
}

Result MCUGPU_SetLcdPower(bool on)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0004, 1, 0); // 0x00040040
	cmdbuf[1] = !!on;
	
	Result res = svcSendSyncRequest(mcuGpuHandle);
	
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUGPU_SetTopLcdFlicker(u8 flicker)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0005, 1, 0); // 0x00050040
	cmdbuf[1] = (u32)flicker;
	
	Result res = svcSendSyncRequest(mcuGpuHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUGPU_GetTopLcdFlicker(u8 *out_flicker)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0006, 0, 0); // 0x00060000
	
	Result res = svcSendSyncRequest(mcuGpuHandle);
	if (R_FAILED(res)) return res;
	
	*out_flicker = cmdbuf[2] & 0xFF;
	return (Result)cmdbuf[1];
}

Result MCUGPU_SetBottomLcdFlicker(u8 flicker)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0007, 1, 0); // 0x00070040
	cmdbuf[1] = (u32)flicker;
	
	Result res = svcSendSyncRequest(mcuGpuHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUGPU_GetBottomLcdFlicker(u8 *out_flicker)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0008, 0, 0); // 0x00080000
	
	Result res = svcSendSyncRequest(mcuGpuHandle);
	if (R_FAILED(res)) return res;
	
	*out_flicker = cmdbuf[2] & 0xFF;
	return (Result)cmdbuf[1];
}

Result MCUGPU_GetFwVerHigh(u8 *out)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0009, 0, 0); // 0x00090000

	Result res = svcSendSyncRequest(mcuGpuHandle);
	if (R_FAILED(res)) return res;

	*out = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCUGPU_GetFwVerLow(u8 *out)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000A, 0, 0); // 0x000A0000

	Result res = svcSendSyncRequest(mcuGpuHandle);
	if (R_FAILED(res)) return res;

	*out = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCUGPU_Set3dLedState(bool state)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000B, 1, 0); // 0x000B0040
	cmdbuf[1] = state;

	Result res = svcSendSyncRequest(mcuGpuHandle);
	if (R_FAILED(res)) return res;

	return (Result)cmdbuf[1];
}

Result MCUGPU_Get3dLedState(bool *out_state)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x000C, 0, 0); // 0x000C0000

	Result res = svcSendSyncRequest(mcuGpuHandle);
	if (R_FAILED(res)) return res;
	
	*out_state = !!cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCUGPU_GetEventHandle(Handle *out_event)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x000D, 0, 0); // 0x000D0000
	
	Result res = svcSendSyncRequest(mcuGpuHandle);
	if (R_FAILED(res)) return res;
	
	*out_event = cmdbuf[3];
	return (Result)cmdbuf[1];
}

Result MCUGPU_GetReceivedEvents(u32 *out_events)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x000E, 0, 0); // 0x000E0000
	
	Result res = svcSendSyncRequest(mcuGpuHandle);
	if (R_FAILED(res)) return res;
	
	*out_events = cmdbuf[2];
	return (Result)cmdbuf[1];
}