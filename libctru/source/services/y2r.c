#include <stdlib.h>
#include <string.h>
#include <3ds/services/y2r.h>
#include <3ds/srv.h>
#include <3ds/svc.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/ipc.h>
#include <3ds/synchronization.h>

Handle y2rHandle;
static int y2rRefCount;

Result y2rInit(void)
{
	Result ret = 0;

	if (AtomicPostIncrement(&y2rRefCount)) return 0;

	ret = srvGetServiceHandle(&y2rHandle, "y2r:u");
	if (R_SUCCEEDED(ret))
	{
		ret = Y2RU_DriverInitialize();
		if (R_FAILED(ret)) svcCloseHandle(y2rHandle);
	}
	if (R_FAILED(ret)) AtomicDecrement(&y2rRefCount);
	return ret;
}

void y2rExit(void)
{
	if (AtomicDecrement(&y2rRefCount)) return;
	Y2RU_DriverFinalize();
	svcCloseHandle(y2rHandle);
}

Result Y2RU_SetInputFormat(Y2RU_InputFormat format)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1,1,0); // 0x10040
	cmdbuf[1] = format;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_GetInputFormat(Y2RU_InputFormat* format)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2,0,0); // 0x20000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*format = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_SetOutputFormat(Y2RU_OutputFormat format)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x3,1,0); // 0x30040
	cmdbuf[1] = format;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_GetOutputFormat(Y2RU_OutputFormat* format)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x4,0,0); // 0x40000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*format = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_SetRotation(Y2RU_Rotation rotation)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x5,1,0); // 0x50040
	cmdbuf[1] = rotation;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_GetRotation(Y2RU_Rotation* rotation)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x6,0,0); // 0x60000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*rotation = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_SetBlockAlignment(Y2RU_BlockAlignment alignment)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x7,1,0); // 0x70040
	cmdbuf[1] = alignment;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_GetBlockAlignment(Y2RU_BlockAlignment* alignment)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x8,0,0); // 0x80000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*alignment = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_SetSpacialDithering(bool enable)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x9,1,0); // 0x90040
	cmdbuf[1] = enable;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_GetSpacialDithering(bool* enabled)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xA,0,0); // 0xA0000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*enabled = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_SetTemporalDithering(bool enable)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xB,1,0); // 0xB0040
	cmdbuf[1] = enable;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_GetTemporalDithering(bool* enabled)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xC,0,0); // 0xC0000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*enabled = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_SetTransferEndInterrupt(bool should_interrupt)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xD,1,0); // 0xD0040
	cmdbuf[1] = should_interrupt;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_GetTransferEndInterrupt(bool* should_interrupt)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xE,0,0); // 0xE0000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*should_interrupt = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_GetTransferEndEvent(Handle* end_event)
{
	if (*end_event != 0)
	{
		svcCloseHandle(*end_event);
		*end_event = 0;
	}

	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xF,0,0); // 0xF0000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;

	*end_event = cmdbuf[3];
	return cmdbuf[1];
}

Result Y2RU_SetSendingY(const void* src_buf, u32 image_size, s16 transfer_unit, s16 transfer_gap)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x10,4,2); // 0x100102
	cmdbuf[1] = (u32)src_buf;
	cmdbuf[2] = image_size;
	cmdbuf[3] = transfer_unit;
	cmdbuf[4] = transfer_gap;
	cmdbuf[5] = IPC_Desc_SharedHandles(1);
	cmdbuf[6] = CUR_PROCESS_HANDLE;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetSendingU(const void* src_buf, u32 image_size, s16 transfer_unit, s16 transfer_gap)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x11,4,2); // 0x110102
	cmdbuf[1] = (u32)src_buf;
	cmdbuf[2] = image_size;
	cmdbuf[3] = transfer_unit;
	cmdbuf[4] = transfer_gap;
	cmdbuf[5] = IPC_Desc_SharedHandles(1);
	cmdbuf[6] = CUR_PROCESS_HANDLE;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetSendingV(const void* src_buf, u32 image_size, s16 transfer_unit, s16 transfer_gap)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x12,4,2); // 0x120102
	cmdbuf[1] = (u32)src_buf;
	cmdbuf[2] = image_size;
	cmdbuf[3] = transfer_unit;
	cmdbuf[4] = transfer_gap;
	cmdbuf[5] = IPC_Desc_SharedHandles(1);
	cmdbuf[6] = CUR_PROCESS_HANDLE;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetSendingYUYV(const void* src_buf, u32 image_size, s16 transfer_unit, s16 transfer_gap)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x13,4,2); // 0x130102
	cmdbuf[1] = (u32)src_buf;
	cmdbuf[2] = image_size;
	cmdbuf[3] = transfer_unit;
	cmdbuf[4] = transfer_gap;
	cmdbuf[5] = IPC_Desc_SharedHandles(1);
	cmdbuf[6] = CUR_PROCESS_HANDLE;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_IsDoneSendingYUYV(bool* is_done)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x14,0,0); // 0x140000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*is_done = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_IsDoneSendingY(bool* is_done)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x15,0,0); // 0x150000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*is_done = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_IsDoneSendingU(bool* is_done)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x16,0,0); // 0x160000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*is_done = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_IsDoneSendingV(bool* is_done)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x17,0,0); // 0x170000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*is_done = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_SetReceiving(void* dst_buf, u32 image_size, s16 transfer_unit, s16 transfer_gap)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x18,4,2); // 0x180102
	cmdbuf[1] = (u32)dst_buf;
	cmdbuf[2] = image_size;
	cmdbuf[3] = transfer_unit;
	cmdbuf[4] = transfer_gap;
	cmdbuf[5] = IPC_Desc_SharedHandles(1);
	cmdbuf[6] = CUR_PROCESS_HANDLE;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_IsDoneReceiving(bool* is_done)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x19,0,0); // 0x190000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*is_done = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_SetInputLineWidth(u16 line_width)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1A,1,0); // 0x1A0040
	cmdbuf[1] = line_width;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_GetInputLineWidth(u16* line_width)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1B,0,0); // 0x1B0000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*line_width = cmdbuf[2] & 0xFFFF;
	return cmdbuf[1];
}

Result Y2RU_SetInputLines(u16 num_lines)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1C,1,0); // 0x1C0040
	cmdbuf[1] = num_lines;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_GetInputLines(u16* num_lines)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1D,0,0); // 0x1D0000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*num_lines = cmdbuf[2] & 0xFFFF;
	return cmdbuf[1];
}

Result Y2RU_SetCoefficients(const Y2RU_ColorCoefficients* coefficients)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1E,4,0); // 0x1E0100
	memcpy(&cmdbuf[1], coefficients, sizeof(Y2RU_ColorCoefficients));

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_GetCoefficients(Y2RU_ColorCoefficients* coefficients)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1F,0,0); // 0x1F0000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	memcpy(coefficients,cmdbuf + 2, sizeof(Y2RU_ColorCoefficients));
	return cmdbuf[1];
}

Result Y2RU_SetStandardCoefficient(Y2RU_StandardCoefficient coefficient)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x20,1,0); // 0x200040
	cmdbuf[1] = coefficient;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_GetStandardCoefficient(Y2RU_ColorCoefficients* coefficients, Y2RU_StandardCoefficient standardCoeff)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x21,1,0); // 0x210040
	cmdbuf[1] = standardCoeff;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	memcpy(coefficients,cmdbuf + 2, sizeof(Y2RU_ColorCoefficients));
	return cmdbuf[1];
}

Result Y2RU_SetAlpha(u16 alpha)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x22,1,0); // 0x220040
	cmdbuf[1] = alpha;

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_GetAlpha(u16* alpha)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x23,0,0); // 0x230000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*alpha = cmdbuf[2] & 0xFFFF;
	return cmdbuf[1];
}


Result Y2RU_SetDitheringWeightParams(const Y2RU_DitheringWeightParams* params)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x24,8,0); // 0x240200
	memcpy(&cmdbuf[1], params, sizeof(Y2RU_DitheringWeightParams));

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_GetDitheringWeightParams(Y2RU_DitheringWeightParams* params)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x25,0,0); // 0x250000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	memcpy(params,cmdbuf+2, sizeof(Y2RU_DitheringWeightParams));
	return cmdbuf[1];
}

Result Y2RU_StartConversion(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x26,0,0); // 0x260000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_StopConversion(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x27,0,0); // 0x270000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_IsBusyConversion(bool* is_busy)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x28,0,0); // 0x280000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*is_busy = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_SetConversionParams(const Y2RU_ConversionParams* params)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x29,7,0); // 0x2901C0
	memcpy(&cmdbuf[1], params, sizeof(Y2RU_ConversionParams));

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_PingProcess(u8* ping)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2A,0,0); // 0x2A0000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	*ping = (u8)cmdbuf[2];
	return cmdbuf[1];
}

Result Y2RU_DriverInitialize(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2B,0,0); // 0x2B0000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}

Result Y2RU_DriverFinalize(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2C,0,0); // 0x2C0000

	if (R_FAILED(ret = svcSendSyncRequest(y2rHandle))) return ret;
	return cmdbuf[1];
}
