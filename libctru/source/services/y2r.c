#include <stdlib.h>
#include <string.h>
#include <3ds/services/y2r.h>
#include <3ds/srv.h>
#include <3ds/svc.h>
#include <3ds/types.h>
#include <3ds/ipc.h>

Handle y2rHandle = 0;
static bool initialized = false;

Result y2rInit(void)
{
	Result ret = 0;

	if (initialized) return 0;

	if (y2rHandle == 0)
	{
		ret = srvGetServiceHandle(&y2rHandle, "y2r:u");
		if (ret < 0) return ret;
	}

	ret = Y2RU_DriverInitialize();
	if (ret < 0) return ret;
	initialized = true;

	return 0;
}

Result y2rExit(void)
{
	Result ret = 0;

	if (initialized)
	{
		ret = Y2RU_DriverFinalize();
		if (ret < 0) return ret;
	}

	if (y2rHandle != 0)
	{
		ret = svcCloseHandle(y2rHandle);
		if (ret < 0) return ret;
		y2rHandle = 0;
	}

	return 0;
}

Result Y2RU_SetInputFormat(Y2R_InputFormat format)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1,1,0); // 0x10040
	cmdbuf[1] = format;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetOutputFormat(Y2R_OutputFormat format)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x3,1,0); // 0x30040
	cmdbuf[1] = format;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetRotation(Y2R_Rotation rotation)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x5,1,0); // 0x50040
	cmdbuf[1] = rotation;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetBlockAlignment(Y2R_BlockAlignment alignment)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x7,1,0); // 0x70040
	cmdbuf[1] = alignment;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetTransferEndInterrupt(bool should_interrupt)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xD,1,0); // 0xD0040
	cmdbuf[1] = should_interrupt;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
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

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;

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

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
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

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
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

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
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

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_IsDoneSendingYUYV(bool* is_done)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x14,0,0); // 0x140000

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	*is_done = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_IsDoneSendingY(bool* is_done)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x15,0,0); // 0x150000

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	*is_done = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_IsDoneSendingU(bool* is_done)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x16,0,0); // 0x160000

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	*is_done = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_IsDoneSendingV(bool* is_done)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x17,0,0); // 0x170000

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
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

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_IsDoneReceiving(bool* is_done)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x19,0,0); // 0x190000

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	*is_done = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_SetInputLineWidth(u16 line_width)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1A,1,0); // 0x1A0040
	cmdbuf[1] = line_width;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetInputLines(u16 num_lines)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1C,1,0); // 0x1C0040
	cmdbuf[1] = num_lines;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetCoefficients(const Y2R_ColorCoefficients* coefficients)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1E,4,0); // 0x1E0100
	memcpy(&cmdbuf[1], coefficients, sizeof(Y2R_ColorCoefficients));

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetStandardCoefficient(Y2R_StandardCoefficient coefficient)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x20,1,0); // 0x200040
	cmdbuf[1] = coefficient;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetAlpha(u16 alpha)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x22,1,0); // 0x220040
	cmdbuf[1] = alpha;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetUnknownParams(const u16 params[16])
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x24,8,0); // 0x240200
	memcpy(&cmdbuf[1], params, sizeof(u16) * 16);

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_StartConversion(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x26,0,0); // 0x260000

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_StopConversion(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x27,0,0); // 0x270000

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_IsBusyConversion(bool* is_busy)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x28,0,0); // 0x280000

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	*is_busy = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_SetConversionParams(const Y2R_ConversionParams* params)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x29,7,0); // 0x2901C0
	memcpy(&cmdbuf[1], params, sizeof(Y2R_ConversionParams));

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_PingProcess(u8* ping)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2A,0,0); // 0x2A0000

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	*ping = (u8)cmdbuf[2];
	return cmdbuf[1];
}

Result Y2RU_DriverInitialize(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2B,0,0); // 0x2B0000

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_DriverFinalize(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2C,0,0); // 0x2C0000

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}
