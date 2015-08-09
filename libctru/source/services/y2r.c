#include <stdlib.h>
#include <string.h>
#include <3ds/services/y2r.h>
#include <3ds/srv.h>
#include <3ds/svc.h>
#include <3ds/types.h>

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
	cmdbuf[0] = 0x00010040;
	cmdbuf[1] = format;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetOutputFormat(Y2R_OutputFormat format)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00030040;
	cmdbuf[1] = format;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetRotation(Y2R_Rotation rotation)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00050040;
	cmdbuf[1] = rotation;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetBlockAlignment(Y2R_BlockAlignment alignment)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00070040;
	cmdbuf[1] = alignment;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetTransferEndInterrupt(bool should_interrupt)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x000D0040;
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
	cmdbuf[0] = 0x000F0000;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;

	*end_event = cmdbuf[3];
	return cmdbuf[1];
}

Result Y2RU_SetSendingY(const void* src_buf, u32 image_size, s16 transfer_unit, s16 transfer_gap)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00100102;
	cmdbuf[1] = (u32)src_buf;
	cmdbuf[2] = image_size;
	cmdbuf[3] = transfer_unit;
	cmdbuf[4] = transfer_gap;
	cmdbuf[5] = 0;
	cmdbuf[6] = 0xFFFF8001;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetSendingU(const void* src_buf, u32 image_size, s16 transfer_unit, s16 transfer_gap)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00110102;
	cmdbuf[1] = (u32)src_buf;
	cmdbuf[2] = image_size;
	cmdbuf[3] = transfer_unit;
	cmdbuf[4] = transfer_gap;
	cmdbuf[5] = 0;
	cmdbuf[6] = 0xFFFF8001;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetSendingV(const void* src_buf, u32 image_size, s16 transfer_unit, s16 transfer_gap)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00120102;
	cmdbuf[1] = (u32)src_buf;
	cmdbuf[2] = image_size;
	cmdbuf[3] = transfer_unit;
	cmdbuf[4] = transfer_gap;
	cmdbuf[5] = 0;
	cmdbuf[6] = 0xFFFF8001;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetSendingYUYV(const void* src_buf, u32 image_size, s16 transfer_unit, s16 transfer_gap)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00130102;
	cmdbuf[1] = (u32)src_buf;
	cmdbuf[2] = image_size;
	cmdbuf[3] = transfer_unit;
	cmdbuf[4] = transfer_gap;
	cmdbuf[5] = 0;
	cmdbuf[6] = 0xFFFF8001;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_IsDoneSendingYUYV(bool* is_done)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00140000;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	*is_done = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_IsDoneSendingY(bool* is_done)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00150000;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	*is_done = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_IsDoneSendingU(bool* is_done)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00160000;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	*is_done = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_IsDoneSendingV(bool* is_done)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00170000;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	*is_done = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_SetReceiving(void* dst_buf, u32 image_size, s16 transfer_unit, s16 transfer_gap)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00180102;
	cmdbuf[1] = (u32)dst_buf;
	cmdbuf[2] = image_size;
	cmdbuf[3] = transfer_unit;
	cmdbuf[4] = transfer_gap;
	cmdbuf[5] = 0;
	cmdbuf[6] = 0xFFFF8001;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_IsDoneReceiving(bool* is_done)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00190000;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	*is_done = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_SetInputLineWidth(u16 line_width)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x001A0040;
	cmdbuf[1] = line_width;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetInputLines(u16 num_lines)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x001C0040;
	cmdbuf[1] = num_lines;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetCoefficients(const Y2R_ColorCoefficients* coefficients)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x001E0100;
	memcpy(&cmdbuf[1], coefficients, sizeof(Y2R_ColorCoefficients));

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetStandardCoefficient(Y2R_StandardCoefficient coefficient)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00200040;
	cmdbuf[1] = coefficient;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetAlpha(u16 alpha)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00220040;
	cmdbuf[1] = alpha;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_SetUnknownParams(const u16 params[16])
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00240200;
	memcpy(&cmdbuf[1], params, sizeof(u16) * 16);

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_StartConversion(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00260000;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_StopConversion(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00270000;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_IsBusyConversion(bool* is_busy)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00280000;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	*is_busy = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result Y2RU_SetConversionParams(const Y2R_ConversionParams* params)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x002901C0;
	memcpy(&cmdbuf[1], params, sizeof(Y2R_ConversionParams));

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_PingProcess(u8* ping)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x002A0000;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	*ping = (u8)cmdbuf[2];
	return cmdbuf[1];
}

Result Y2RU_DriverInitialize(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x002B0000;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result Y2RU_DriverFinalize(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x002C0000;

	if ((ret = svcSendSyncRequest(y2rHandle)) != 0) return ret;
	return cmdbuf[1];
}
