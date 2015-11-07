#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/ipc.h>
#include <3ds/synchronization.h>
#include <3ds/services/dsp.h>

static Handle dspHandle;
static int dspRefCount;

Result dspInit(void)
{
	Result ret = 0;

	if (AtomicPostIncrement(&dspRefCount)) return 0;

	ret = srvGetServiceHandle(&dspHandle, "dsp::DSP");
	if (R_SUCCEEDED(ret)) DSP_UnloadComponent();
	else                  AtomicDecrement(&dspRefCount);

	return ret;
}

void dspExit(void)
{
	if (AtomicDecrement(&dspRefCount)) return;
	svcCloseHandle(dspHandle);
}

Result DSP_GetHeadphoneStatus(bool* is_inserted)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1F,0,0);
	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;
	*is_inserted = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result DSP_FlushDataCache(const void* address, u32 size)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x13,2,2);
	cmdbuf[1] = (u32)address;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = CUR_PROCESS_HANDLE;
	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;
	return cmdbuf[1];
}

Result DSP_InvalidateDataCache(const void* address, u32 size)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x14,2,2);
	cmdbuf[1] = (u32)address;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = CUR_PROCESS_HANDLE;
	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;
	return cmdbuf[1];
}

Result DSP_SetSemaphore(u16 value)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x7,1,0);
	cmdbuf[1] = value;
	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;
	return cmdbuf[1];
}

Result DSP_SetSemaphoreMask(u16 mask)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x17,1,0);
	cmdbuf[1] = mask;
	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;
	return cmdbuf[1];
}

Result DSP_GetSemaphoreHandle(Handle* semaphore)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x16,0,0);
	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;
	*semaphore = cmdbuf[3];
	return cmdbuf[1];
}

Result DSP_LoadComponent(const void* component, u32 size, u16 prog_mask, u16 data_mask, bool* is_loaded)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x11,3,2);
	cmdbuf[1] = size;
	cmdbuf[2] = prog_mask;
	cmdbuf[3] = data_mask;
	cmdbuf[4] = IPC_Desc_Buffer(size,IPC_BUFFER_R);
	cmdbuf[5] = (u32) component;
	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;
	*is_loaded = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result DSP_UnloadComponent(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x12,0,0);
	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;
	return cmdbuf[1];
}

Result DSP_RegisterInterruptEvents(Handle handle, u32 interrupt, u32 channel)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x15,2,2);
	cmdbuf[1] = interrupt;
	cmdbuf[2] = channel;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = handle;
	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;
	return cmdbuf[1];
}

Result DSP_ReadPipeIfPossible(u32 channel, u32 peer, void* buffer, u16 length, u16* length_read)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x10,3,0);
	cmdbuf[1] = channel;
	cmdbuf[2] = peer;
	cmdbuf[3] = length;

	u32 * staticbufs = getThreadStaticBuffers();

	u32 saved1 = staticbufs[0];
	u32 saved2 = staticbufs[1];

	staticbufs[0] = IPC_Desc_StaticBuffer(length,0);
	staticbufs[1] = (u32)buffer;

	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;

	staticbufs[0] = saved1;
	staticbufs[1] = saved2;

	if (length_read)
		*length_read = cmdbuf[2] & 0xFFFF;
	return cmdbuf[1];
}

Result DSP_WriteProcessPipe(u32 channel, const void* buffer, u32 length)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xD,2,2);
	cmdbuf[1] = channel;
	cmdbuf[2] = length;
	cmdbuf[3] = IPC_Desc_StaticBuffer(length,1);
	cmdbuf[4] = (u32)buffer;
	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;
	return cmdbuf[1];
}

Result DSP_ConvertProcessAddressFromDspDram(u32 dsp_address, u32* arm_address)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xC,1,0);
	cmdbuf[1] = dsp_address;
	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;
	*arm_address = cmdbuf[2];
	return cmdbuf[1];
}

Result DSP_RecvData(u16 regNo, u16* value)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1,1,0);
	cmdbuf[1] = regNo;
	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;
	*value = cmdbuf[2] & 0xFFFF;
	return cmdbuf[1];
}

Result DSP_RecvDataIsReady(u16 regNo, bool* is_ready)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2,1,0);
	cmdbuf[1] = regNo;
	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;
	*is_ready = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

// Writes data to the reg regNo
// *(_WORD *)(8 * regNo + 0x1ED03024) = value
Result DSP_SendData(u16 regNo, u16 value)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x3,2,0);
	cmdbuf[1] = regNo;
	cmdbuf[2] = value;
	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;
	return cmdbuf[1];
}

Result DSP_SendDataIsEmpty(u16 regNo, bool* is_empty)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x4,1,0);
	cmdbuf[1] = regNo;
	if (R_FAILED(ret = svcSendSyncRequest(dspHandle))) return ret;
	*is_empty = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}
