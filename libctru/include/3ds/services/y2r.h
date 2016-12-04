/**
 * @file y2r.h
 * @brief Y2R service for hardware YUV->RGB conversions
 */
#pragma once
#include <3ds/types.h>

/**
 * @brief Input color formats
 *
 * For the 16-bit per component formats, bits 15-8 are padding and 7-0 contains the value.
 */
typedef enum
{
	INPUT_YUV422_INDIV_8  = 0x0, ///<  8-bit per component, planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples).\n Usually named YUV422P.
	INPUT_YUV420_INDIV_8  = 0x1, ///<  8-bit per component, planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples).\n Usually named YUV420P.
	INPUT_YUV422_INDIV_16 = 0x2, ///< 16-bit per component, planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples).\n Usually named YUV422P16.
	INPUT_YUV420_INDIV_16 = 0x3, ///< 16-bit per component, planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples).\n Usually named YUV420P16.
	INPUT_YUV422_BATCH    = 0x4, ///<  8-bit per component, packed YUV 4:2:2, 16bpp, (Y0 Cb Y1 Cr).\n Usually named YUYV422.
} Y2RU_InputFormat;

/**
 * @brief Output color formats
 *
 * Those are the same as the framebuffer and GPU texture formats.
 */
typedef enum
{
	OUTPUT_RGB_32     = 0x0, ///< 32-bit RGBA8888. The alpha component is the 8-bit value set by @ref Y2RU_SetAlpha
	OUTPUT_RGB_24     = 0x1, ///< 24-bit RGB888.
	OUTPUT_RGB_16_555 = 0x2, ///< 16-bit RGBA5551. The alpha bit is the 7th bit of the alpha value set by @ref Y2RU_SetAlpha
	OUTPUT_RGB_16_565 = 0x3, ///< 16-bit RGB565.
} Y2RU_OutputFormat;

/// Rotation to be applied to the output.
typedef enum
{
	ROTATION_NONE          = 0x0, ///< No rotation.
	ROTATION_CLOCKWISE_90  = 0x1, ///< Clockwise 90 degrees.
	ROTATION_CLOCKWISE_180 = 0x2, ///< Clockwise 180 degrees.
	ROTATION_CLOCKWISE_270 = 0x3, ///< Clockwise 270 degrees.
} Y2RU_Rotation;

/**
 * @brief Block alignment of output
 *
 * Defines the way the output will be laid out in memory.
 */
typedef enum
{
	BLOCK_LINE   = 0x0, ///< The result buffer will be laid out in linear format, the usual way.
	BLOCK_8_BY_8 = 0x1, ///< The result will be stored as 8x8 blocks in Z-order.\n Useful for textures since it is the format used by the PICA200.
} Y2RU_BlockAlignment;

/**
 * @brief Coefficients of the YUV->RGB conversion formula.
 *
 * A set of coefficients configuring the RGB to YUV conversion. Coefficients 0-4 are unsigned 2.8
 * fixed pointer numbers representing entries on the conversion matrix, while coefficient 5-7 are
 * signed 11.5 fixed point numbers added as offsets to the RGB result.
 *
 * The overall conversion process formula is:
 * @code
 * R = trunc((rgb_Y * Y           + r_V * V) + 0.75 + r_offset)
 * G = trunc((rgb_Y * Y - g_U * U - g_V * V) + 0.75 + g_offset)
 * B = trunc((rgb_Y * Y + b_U * U          ) + 0.75 + b_offset)
 * @endcode
 */
typedef struct
{
	u16 rgb_Y;    ///< RGB per unit Y.
	u16 r_V;      ///< Red per unit V.
	u16 g_V;      ///< Green per unit V.
	u16 g_U;      ///< Green per unit U.
	u16 b_U;      ///< Blue per unit U.
	u16 r_offset; ///< Red offset.
	u16 g_offset; ///< Green offset.
	u16 b_offset; ///< Blue offset.
} Y2RU_ColorCoefficients;

/**
 * @brief Preset conversion coefficients based on ITU standards for the YUV->RGB formula.
 *
 * For more details refer to @ref Y2RU_ColorCoefficients
 */
typedef enum
{
	COEFFICIENT_ITU_R_BT_601         = 0x0, ///< Coefficients from the ITU-R BT.601 standard with PC ranges.
	COEFFICIENT_ITU_R_BT_709         = 0x1, ///< Coefficients from the ITU-R BT.709 standard with PC ranges.
	COEFFICIENT_ITU_R_BT_601_SCALING = 0x2, ///< Coefficients from the ITU-R BT.601 standard with TV ranges.
	COEFFICIENT_ITU_R_BT_709_SCALING = 0x3, ///< Coefficients from the ITU-R BT.709 standard with TV ranges.
} Y2RU_StandardCoefficient;

/**
 * @brief Structure used to configure all parameters at once.
 *
 * You can send a batch of configuration parameters using this structure and @ref Y2RU_SetConversionParams.
 */
typedef struct
{
	Y2RU_InputFormat input_format       : 8;           ///< Value passed to @ref Y2RU_SetInputFormat
	Y2RU_OutputFormat output_format     : 8;           ///< Value passed to @ref Y2RU_SetOutputFormat
	Y2RU_Rotation rotation              : 8;           ///< Value passed to @ref Y2RU_SetRotation
	Y2RU_BlockAlignment block_alignment : 8;           ///< Value passed to @ref Y2RU_SetBlockAlignment
	s16 input_line_width;                              ///< Value passed to @ref Y2RU_SetInputLineWidth
	s16 input_lines;                                   ///< Value passed to @ref Y2RU_SetInputLines
	Y2RU_StandardCoefficient standard_coefficient : 8; ///< Value passed to @ref Y2RU_SetStandardCoefficient
	u8 unused;                                         ///< Unused.
	u16 alpha;                                         ///< Value passed to @ref Y2RU_SetAlpha
} Y2RU_ConversionParams;

/// Dithering weights.
typedef struct
{
	u16 w0_xEven_yEven; ///< Weight 0 for even X, even Y.
	u16 w0_xOdd_yEven;  ///< Weight 0 for odd X, even Y.
	u16 w0_xEven_yOdd;  ///< Weight 0 for even X, odd Y.
	u16 w0_xOdd_yOdd;   ///< Weight 0 for odd X, odd Y.
	u16 w1_xEven_yEven; ///< Weight 1 for even X, even Y.
	u16 w1_xOdd_yEven;  ///< Weight 1 for odd X, even Y.
	u16 w1_xEven_yOdd;  ///< Weight 1 for even X, odd Y.
	u16 w1_xOdd_yOdd;   ///< Weight 1 for odd X, odd Y.
	u16 w2_xEven_yEven; ///< Weight 2 for even X, even Y.
	u16 w2_xOdd_yEven;  ///< Weight 2 for odd X, even Y.
	u16 w2_xEven_yOdd;  ///< Weight 2 for even X, odd Y.
	u16 w2_xOdd_yOdd;   ///< Weight 2 for odd X, odd Y.
	u16 w3_xEven_yEven; ///< Weight 3 for even X, even Y.
	u16 w3_xOdd_yEven;  ///< Weight 3 for odd X, even Y.
	u16 w3_xEven_yOdd;  ///< Weight 3 for even X, odd Y.
	u16 w3_xOdd_yOdd;   ///< Weight 3 for odd X, odd Y.
} Y2RU_DitheringWeightParams;

/**
 * @brief Initializes the y2r service.
 *
 * This will internally get the handle of the service, and on success call Y2RU_DriverInitialize.
 */
Result y2rInit(void);

/**
 * @brief Closes the y2r service.
 *
 * This will internally call Y2RU_DriverFinalize and close the handle of the service.
 */
void y2rExit(void);

/**
 * @brief Used to configure the input format.
 * @param format Input format to use.
 *
 * @note Prefer using @ref Y2RU_SetConversionParams if you have to set multiple parameters.
 */
Result Y2RU_SetInputFormat(Y2RU_InputFormat format);

/**
 * @brief Gets the configured input format.
 * @param format Pointer to output the input format to.
 */
Result Y2RU_GetInputFormat(Y2RU_InputFormat* format);

/**
 * @brief Used to configure the output format.
 * @param format Output format to use.
 *
 * @note Prefer using @ref Y2RU_SetConversionParams if you have to set multiple parameters.
 */
Result Y2RU_SetOutputFormat(Y2RU_OutputFormat format);

/**
 * @brief Gets the configured output format.
 * @param format Pointer to output the output format to.
 */
Result Y2RU_GetOutputFormat(Y2RU_OutputFormat* format);

/**
 * @brief Used to configure the rotation of the output.
 * @param rotation Rotation to use.
 *
 * It seems to apply the rotation per batch of 8 lines, so the output will be (height/8) images of size 8 x width.
 *
 * @note Prefer using @ref Y2RU_SetConversionParams if you have to set multiple parameters.
 */
Result Y2RU_SetRotation(Y2RU_Rotation rotation);

/**
 * @brief Gets the configured rotation.
 * @param rotation Pointer to output the rotation to.
 */
Result Y2RU_GetRotation(Y2RU_Rotation* rotation);

/**
 * @brief Used to configure the alignment of the output buffer.
 * @param alignment Alignment to use.
 *
 * @note Prefer using @ref Y2RU_SetConversionParams if you have to set multiple parameters.
 */
Result Y2RU_SetBlockAlignment(Y2RU_BlockAlignment alignment);

/**
 * @brief Gets the configured alignment.
 * @param alignment Pointer to output the alignment to.
 */
Result Y2RU_GetBlockAlignment(Y2RU_BlockAlignment* alignment);

/**
 * @brief Sets whether to use spacial dithering.
 * @param enable Whether to use spacial dithering.
 */
Result Y2RU_SetSpacialDithering(bool enable);

/**
 * @brief Gets whether to use spacial dithering.
 * @param enable Pointer to output the spacial dithering state to.
 */
Result Y2RU_GetSpacialDithering(bool* enabled);

/**
 * @brief Sets whether to use temporal dithering.
 * @param enable Whether to use temporal dithering.
 */
Result Y2RU_SetTemporalDithering(bool enable);

/**
 * @brief Gets whether to use temporal dithering.
 * @param enable Pointer to output the temporal dithering state to.
 */
Result Y2RU_GetTemporalDithering(bool* enabled);


/**
 * @brief Used to configure the width of the image.
 * @param line_width Width of the image in pixels. Must be a multiple of 8, up to 1024.
 *
 * @note Prefer using @ref Y2RU_SetConversionParams if you have to set multiple parameters.
 */
Result Y2RU_SetInputLineWidth(u16 line_width);

/**
 * @brief Gets the configured input line width.
 * @param line_width Pointer to output the line width to.
 */
Result Y2RU_GetInputLineWidth(u16* line_width);

/**
 * @brief Used to configure the height of the image.
 * @param num_lines Number of lines to be converted.
 *
 * A multiple of 8 seems to be preferred.
 * If using the @ref BLOCK_8_BY_8 mode, it must be a multiple of 8.
 *
 * @note Prefer using @ref Y2RU_SetConversionParams if you have to set multiple parameters.
 */
Result Y2RU_SetInputLines(u16 num_lines);

/**
 * @brief Gets the configured number of input lines.
 * @param num_lines Pointer to output the input lines to.
 */
Result Y2RU_GetInputLines(u16* num_lines);

/**
 * @brief Used to configure the color conversion formula.
 * @param coefficients Coefficients to use.
 *
 * See @ref Y2RU_ColorCoefficients for more information about the coefficients.
 *
 * @note Prefer using @ref Y2RU_SetConversionParams if you have to set multiple parameters.
 */
Result Y2RU_SetCoefficients(const Y2RU_ColorCoefficients* coefficients);

/**
 * @brief Gets the configured color coefficients.
 * @param num_lines Pointer to output the coefficients to.
 */
Result Y2RU_GetCoefficients(Y2RU_ColorCoefficients* coefficients);

/**
 * @brief Used to configure the color conversion formula with ITU stantards coefficients.
 * @param coefficient Standard coefficient to use.
 *
 * See @ref Y2RU_ColorCoefficients for more information about the coefficients.
 *
 * @note Prefer using @ref Y2RU_SetConversionParams if you have to set multiple parameters.
 */
Result Y2RU_SetStandardCoefficient(Y2RU_StandardCoefficient coefficient);

/**
 * @brief Gets the color coefficient parameters of a standard coefficient.
 * @param coefficients Pointer to output the coefficients to.
 * @param standardCoeff Standard coefficient to check.
 */
Result Y2RU_GetStandardCoefficient(Y2RU_ColorCoefficients* coefficients, Y2RU_StandardCoefficient standardCoeff);

/**
 * @brief Used to configure the alpha value of the output.
 * @param alpha 8-bit value to be used for the output when the format requires it.
 *
 * @note Prefer using @ref Y2RU_SetConversionParams if you have to set multiple parameters.
 */
Result Y2RU_SetAlpha(u16 alpha);

/**
 * @brief Gets the configured output alpha value.
 * @param alpha Pointer to output the alpha value to.
 */
Result Y2RU_GetAlpha(u16* alpha);

/**
 * @brief Used to enable the end of conversion interrupt.
 * @param should_interrupt Enables the interrupt if true, disable it if false.
 *
 * It is possible to fire an interrupt when the conversion is finished, and that the DMA is done copying the data.
 * This interrupt will then be used to fire an event. See @ref Y2RU_GetTransferEndEvent.
 * By default the interrupt is enabled.
 *
 * @note It seems that the event can be fired too soon in some cases, depending the transfer_unit size.\n Please see the note at @ref Y2RU_SetReceiving
 */
Result Y2RU_SetTransferEndInterrupt(bool should_interrupt);

/**
 * @brief Gets whether the transfer end interrupt is enabled.
 * @param should_interrupt Pointer to output the interrupt state to.
 */
Result Y2RU_GetTransferEndInterrupt(bool* should_interrupt);

/**
 * @brief Gets an handle to the end of conversion event.
 * @param end_event Pointer to the event handle to be set to the end of conversion event. It isn't necessary to create or close this handle.
 *
 * To enable this event you have to use @code{C} Y2RU_SetTransferEndInterrupt(true);@endcode
 * The event will be triggered when the corresponding interrupt is fired.
 *
 * @note It is recommended to use a timeout when waiting on this event, as it sometimes (but rarely) isn't triggered.
 */
Result Y2RU_GetTransferEndEvent(Handle* end_event);

/**
 * @brief Configures the Y plane buffer.
 * @param src_buf A pointer to the beginning of your Y data buffer.
 * @param image_size The total size of the data buffer.
 * @param transfer_unit Specifies the size of 1 DMA transfer. Usually set to 1 line. This has to be a divisor of image_size.
 * @param transfer_gap Specifies the gap (offset) to be added after each transfer. Can be used to convert images with stride or only a part of it.
 *
 * @warning transfer_unit+transfer_gap must be less than 32768 (0x8000)
 *
 * This specifies the Y data buffer for the planar input formats (INPUT_YUV42*_INDIV_*).
 * The actual transfer will only happen after calling @ref Y2RU_StartConversion.
 */
Result Y2RU_SetSendingY(const void* src_buf, u32 image_size, s16 transfer_unit, s16 transfer_gap);

/**
 * @brief Configures the U plane buffer.
 * @param src_buf A pointer to the beginning of your Y data buffer.
 * @param image_size The total size of the data buffer.
 * @param transfer_unit Specifies the size of 1 DMA transfer. Usually set to 1 line. This has to be a divisor of image_size.
 * @param transfer_gap Specifies the gap (offset) to be added after each transfer. Can be used to convert images with stride or only a part of it.
 *
 * @warning transfer_unit+transfer_gap must be less than 32768 (0x8000)
 *
 * This specifies the U data buffer for the planar input formats (INPUT_YUV42*_INDIV_*).
 * The actual transfer will only happen after calling @ref Y2RU_StartConversion.
 */
Result Y2RU_SetSendingU(const void* src_buf, u32 image_size, s16 transfer_unit, s16 transfer_gap);

/**
 * @brief Configures the V plane buffer.
 * @param src_buf A pointer to the beginning of your Y data buffer.
 * @param image_size The total size of the data buffer.
 * @param transfer_unit Specifies the size of 1 DMA transfer. Usually set to 1 line. This has to be a divisor of image_size.
 * @param transfer_gap Specifies the gap (offset) to be added after each transfer. Can be used to convert images with stride or only a part of it.
 *
 * @warning transfer_unit+transfer_gap must be less than 32768 (0x8000)
 *
 * This specifies the V data buffer for the planar input formats (INPUT_YUV42*_INDIV_*).
 * The actual transfer will only happen after calling @ref Y2RU_StartConversion.
 */
Result Y2RU_SetSendingV(const void* src_buf, u32 image_size, s16 transfer_unit, s16 transfer_gap);

/**
 * @brief Configures the YUYV source buffer.
 * @param src_buf A pointer to the beginning of your Y data buffer.
 * @param image_size The total size of the data buffer.
 * @param transfer_unit Specifies the size of 1 DMA transfer. Usually set to 1 line. This has to be a divisor of image_size.
 * @param transfer_gap Specifies the gap (offset) to be added after each transfer. Can be used to convert images with stride or only a part of it.
 *
 * @warning transfer_unit+transfer_gap must be less than 32768 (0x8000)
 *
 * This specifies the YUYV data buffer for the packed input format @ref INPUT_YUV422_BATCH.
 * The actual transfer will only happen after calling @ref Y2RU_StartConversion.
 */
Result Y2RU_SetSendingYUYV(const void* src_buf, u32 image_size, s16 transfer_unit, s16 transfer_gap);

/**
 * @brief Configures the destination buffer.
 * @param src_buf A pointer to the beginning of your destination buffer in FCRAM
 * @param image_size The total size of the data buffer.
 * @param transfer_unit Specifies the size of 1 DMA transfer. Usually set to 1 line. This has to be a divisor of image_size.
 * @param transfer_gap Specifies the gap (offset) to be added after each transfer. Can be used to convert images with stride or only a part of it.
 *
 * This specifies the destination buffer of the conversion.
 * The actual transfer will only happen after calling @ref Y2RU_StartConversion.
 * The buffer does NOT need to be allocated in the linear heap.
 *
 * @warning transfer_unit+transfer_gap must be less than 32768 (0x8000)
 *
 * @note
 *      It seems that depending on the size of the image and of the transfer unit,\n
 *      it is possible for the end of conversion interrupt to be triggered right after the conversion began.\n
 *      One line as transfer_unit seems to trigger this issue for 400x240, setting to 2/4/8 lines fixes it.
 *
 * @note Setting a transfer_unit of 4 or 8 lines seems to bring the best results in terms of speed for a 400x240 image.
 */
Result Y2RU_SetReceiving(void* dst_buf, u32 image_size, s16 transfer_unit, s16 transfer_gap);

/**
 * @brief Checks if the DMA has finished sending the Y buffer.
 * @param is_done Pointer to the boolean that will hold the result.
 *
 * True if the DMA has finished transferring the Y plane, false otherwise. To be used with @ref Y2RU_SetSendingY.
 */
Result Y2RU_IsDoneSendingY(bool* is_done);

/**
 * @brief Checks if the DMA has finished sending the U buffer.
 * @param is_done Pointer to the boolean that will hold the result.
 *
 * True if the DMA has finished transferring the U plane, false otherwise. To be used with @ref Y2RU_SetSendingU.
 */
Result Y2RU_IsDoneSendingU(bool* is_done);

/**
 * @brief Checks if the DMA has finished sending the V buffer.
 * @param is_done Pointer to the boolean that will hold the result.
 *
 * True if the DMA has finished transferring the V plane, false otherwise. To be used with @ref Y2RU_SetSendingV.
 */
Result Y2RU_IsDoneSendingV(bool* is_done);

/**
 * @brief Checks if the DMA has finished sending the YUYV buffer.
 * @param is_done Pointer to the boolean that will hold the result.
 *
 * True if the DMA has finished transferring the YUYV buffer, false otherwise. To be used with @ref Y2RU_SetSendingYUYV.
 */
Result Y2RU_IsDoneSendingYUYV(bool* is_done);

/**
 * @brief Checks if the DMA has finished sending the converted result.
 * @param is_done Pointer to the boolean that will hold the result.
 *
 * True if the DMA has finished transferring data to your destination buffer, false otherwise.
 */
Result Y2RU_IsDoneReceiving(bool* is_done);

/**
 * @brief Configures the dithering weight parameters.
 * @param params Dithering weight parameters to use.
 */
Result Y2RU_SetDitheringWeightParams(const Y2RU_DitheringWeightParams* params);

/**
 * @brief Gets the configured dithering weight parameters.
 * @param params Pointer to output the dithering weight parameters to.
 */
Result Y2RU_GetDitheringWeightParams(Y2RU_DitheringWeightParams* params);

/**
 * @brief Sets all of the parameters of Y2RU_ConversionParams at once.
 * @param params Conversion parameters to set.
 *
 * Faster than calling the individual value through Y2R_Set* because only one system call is made.
 */
Result Y2RU_SetConversionParams(const Y2RU_ConversionParams* params);

/// Starts the conversion process
Result Y2RU_StartConversion(void);

/// Cancels the conversion
Result Y2RU_StopConversion(void);

/**
 * @brief Checks if the conversion and DMA transfer are finished.
 * @param is_busy Pointer to output the busy state to.
 *
 * This can have the same problems as the event and interrupt. See @ref Y2RU_SetTransferEndInterrupt.
 */
Result Y2RU_IsBusyConversion(bool* is_busy);

/**
 * @brief Checks whether Y2R is ready to be used.
 * @param ping Pointer to output the ready status to.
 */
Result Y2RU_PingProcess(u8* ping);

/// Initializes the Y2R driver.
Result Y2RU_DriverInitialize(void);

/// Terminates the Y2R driver.
Result Y2RU_DriverFinalize(void);

