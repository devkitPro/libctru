#pragma once
#include <3ds/types.h>

typedef enum
{
	INPUT_YUV422_INDIV_8 = 0x0,
	INPUT_YUV420_INDIV_8 = 0x1,
	INPUT_YUV422_INDIV_16 = 0x2,
	INPUT_YUV420_INDIV_16 = 0x3,
	INPUT_YUV422_BATCH = 0x4,
} Y2R_InputFormat;

typedef enum
{
	OUTPUT_RGB_32 = 0x0,
	OUTPUT_RGB_24 = 0x1,
	OUTPUT_RGB_16_555 = 0x2,
	OUTPUT_RGB_16_565 = 0x3,
} Y2R_OutputFormat;

typedef enum
{
	ROTATION_NONE = 0x0,
	ROTATION_CLOCKWISE_90 = 0x1,
	ROTATION_CLOCKWISE_180 = 0x2,
	ROTATION_CLOCKWISE_270 = 0x3,
} Y2R_Rotation;

typedef enum
{
	BLOCK_LINE = 0x0,
	BLOCK_8_BY_8 = 0x1,
} Y2R_BlockAlignment;

typedef enum
{
	COEFFICIENT_ITU_R_BT_601 = 0x0,
	COEFFICIENT_ITU_R_BT_709 = 0x1,
	COEFFICIENT_ITU_R_BT_601_SCALING = 0x2,
	COEFFICIENT_ITU_R_BT_709_SCALING = 0x3,
} Y2R_StandardCoefficient;

typedef struct
{
	Y2R_InputFormat input_format : 8;
	Y2R_OutputFormat output_format : 8;
	Y2R_Rotation rotation : 8;
	Y2R_BlockAlignment block_alignment : 8;
	u16 input_line_width;
	u16 input_lines;
	Y2R_StandardCoefficient standard_coefficient : 8;
	u8 unused;
	u16 alpha;
} Y2R_ConversionParams;

/**
 * A set of coefficients configuring the RGB to YUV conversion. Coefficients 0-4 are unsigned 2.8
 * fixed pointer numbers representing entries on the conversion matrix, while coefficient 5-7 are
 * signed 11.5 fixed point numbers added as offsets to the RGB result.
 *
 * The overall conversion process formula is:
 * ```
 * R = trunc((rgb_Y * Y           + r_V * V) + 0.75 + r_offset)
 * G = trunc((rgb_Y * Y - g_U * U - g_V * V) + 0.75 + g_offset)
 * B = trunc((rgb_Y * Y + b_U * U          ) + 0.75 + b_offset)
 * ```
 */
typedef struct
{
	u16 rgb_Y;
	u16 r_V;
	u16 g_V;
	u16 g_U;
	u16 b_U;
	u16 r_offset;
	u16 g_offset;
	u16 b_offset;
} Y2R_ColorCoefficients;

Result y2rInit();
Result y2rExit();

Result Y2RU_SetInputFormat(Y2R_InputFormat format);
Result Y2RU_SetOutputFormat(Y2R_OutputFormat format);
Result Y2RU_SetRotation(Y2R_Rotation rotation);
Result Y2RU_SetBlockAlignment(Y2R_BlockAlignment alignment);

Result Y2RU_SetTransferEndInterrupt(bool should_interrupt);
Result Y2RU_GetTransferEndEvent(Handle* end_event);
Result Y2RU_SetSendingY(const void* src_buf, u32 image_size, u16 transfer_unit, u16 transfer_gap);
Result Y2RU_SetSendingU(const void* src_buf, u32 image_size, u16 transfer_unit, u16 transfer_gap);
Result Y2RU_SetSendingV(const void* src_buf, u32 image_size, u16 transfer_unit, u16 transfer_gap);
Result Y2RU_SetSendingYUYV(const void* src_buf, u32 image_size, u16 transfer_unit, u16 transfer_gap);
Result Y2RU_IsDoneSendingYUYV(bool* is_done);
Result Y2RU_IsDoneSendingY(bool* is_done);
Result Y2RU_IsDoneSendingU(bool* is_done);
Result Y2RU_IsDoneSendingV(bool* is_done);
Result Y2RU_SetReceiving(void* dst_buf, u32 image_size, u16 transfer_unit, u16 transfer_gap);
Result Y2RU_IsDoneReceiving(bool* is_done);
Result Y2RU_SetInputLineWidth(u16 line_width);
Result Y2RU_SetInputLines(u16 num_lines);
Result Y2RU_SetCoefficient(const Y2R_ColorCoefficients* coefficient);
Result Y2RU_SetStandardCoefficient(Y2R_StandardCoefficient coefficient);

Result Y2RU_SetAlpha(u16 alpha);
Result Y2RU_SetUnknownParams(const u16 params[16]);
Result Y2RU_StartConversion(void);
Result Y2RU_StopConversion(void);
Result Y2RU_IsBusyConversion(bool* is_busy);
Result Y2RU_SetConversionParams(const Y2R_ConversionParams* params);

/* Seems to check whether y2r is ready to be used */
Result Y2RU_PingProcess(u8* ping);

Result Y2RU_DriverInitialize(void);
Result Y2RU_DriverFinalize(void);

