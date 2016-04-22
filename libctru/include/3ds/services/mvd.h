/**
 * @file mvd.h
 * @brief MVD service.
 */
#pragma once

//New3DS-only, see also: http://3dbrew.org/wiki/MVD_Services

///These values are the data returned as "result-codes" by MVDSTD.
#define MVD_STATUS_OK 0x17000
#define MVD_STATUS_BUSY 0x17002

/// Processing mode.
typedef enum {
	MVDMODE_COLORFORMATCONV, ///< Converting color formats.
	MVDMODE_VIDEOPROCESSING  ///< Processing video.
} MVDSTD_Mode;

/// Input format.
typedef enum {
	MVD_INPUT_YUYV422 = 0x00010001, ///< YUYV422
	MVD_INPUT_H264 = 0x00020001     ///< H264
} MVDSTD_InputFormat;

/// Output format.
typedef enum {
	MVD_OUTPUT_BGR565 = 0x00040002, ///< BGR565
	MVD_OUTPUT_RGB565 = 0x00040004 ///< RGB565
} MVDSTD_OutputFormat;

/// Processing configuration.
typedef struct {
	MVDSTD_InputFormat input_type;      ///< Input type.
	u32 unk_x04;                     ///< Unknown.
	u32 unk_x08;                     ///< Unknown.
	u32 inwidth;                     ///< Input width.
	u32 inheight;                    ///< Input height.
	u32 physaddr_colorconv_indata;   ///< Physical address of color conversion input data.
	u32 physaddr_colorconv_unk0;     ///< Physical address used with color conversion.
	u32 physaddr_colorconv_unk1;     ///< Physical address used with color conversion.
	u32 physaddr_colorconv_unk2;     ///< Physical address used with color conversion.
	u32 physaddr_colorconv_unk3;     ///< Physical address used with color conversion.
	u32 unk_x28[0x18>>2];            ///< Unknown.
	u32 enable_cropping;             ///< Enables cropping with the input image when non-zero via the following 4 words.
	u32 input_crop_x_pos;
	u32 input_crop_y_pos;
	u32 input_crop_height;
	u32 input_crop_width;
	u32 unk_x54;                     ///< Unknown.
	MVDSTD_OutputFormat output_type;    ///< Output type.
	u32 outwidth;                   ///< Output width.
	u32 outheight;                  ///< Output height.
	u32 physaddr_outdata0;           ///< Physical address of output data.
	u32 physaddr_outdata1_colorconv; ///< Physical address of color conversion output data.
	u32 unk_x6c[0x98>>2];            ///< Unknown.
	u32 flag_x104;                   ///< This enables using the following 4 words when non-zero.
	u32 output_x_pos;                ///< Output X position in the output buffer.
	u32 output_y_pos;                ///< Same as above except for the Y pos.
	u32 output_width_override;       ///< Used for aligning the output width when larger than the output width. Overrides the output width when smaller than the output width.
	u32 output_height_override;      ///< Same as output_width_override except for the output height.
	u32 unk_x118;
} MVDSTD_Config;

/**
 * @brief Initializes MVDSTD. Video processing / H.264 currently isn't supported.
 * @param mode Mode to initialize MVDSTD to.
 * @param input_type Type of input to process.
 * @param output_type Type of output to produce.
 * @param size Size of data to process. Not used when type == MVDTYPE_COLORFORMATCONV.
 */
Result mvdstdInit(MVDSTD_Mode mode, MVDSTD_InputFormat input_type, MVDSTD_OutputFormat output_type, u32 size);

/// Shuts down MVDSTD.
void mvdstdExit(void);

/**
 * @brief Generates a default MVDSTD configuration.
 * @param config Pointer to output the generated config to.
 * @param input_width Input width.
 * @param input_height Input height.
 * @param output_width Output width.
 * @param output_height Output height.
 * @param vaddr_colorconv_indata Virtual address of the color conversion input data.
 * @param vaddr_outdata0 Virtual address of the output data.
 * @param vaddr_outdata1_colorconv Virtual address of the color conversion output data.
 */
void mvdstdGenerateDefaultConfig(MVDSTD_Config*config, u32 input_width, u32 input_height, u32 output_width, u32 output_height, u32 *vaddr_colorconv_indata, u32 *vaddr_outdata0, u32 *vaddr_outdata1_colorconv);

/**
 * @brief Run color-format-conversion.
 * @param config Pointer to the configuration to use.
 */
Result mvdstdConvertImage(MVDSTD_Config* config);

/**
 * @brief Processes a video frame(specifically a NAL-unit).
 * @param inbuf_vaddr Input NAL-unit starting with the 3-byte "00 00 01" prefix. Must be located in linearmem.
 * @param size Size of the input buffer.
 */
Result mvdstdProcessVideoFrame(void* inbuf_vaddr, size_t size);

/**
 * @brief Renders the video frame.
 * @param config Optional pointer to the configuration to use. When NULL, MVDSTD_SetConfig() should have been used previously for this video.
 * @param wait When true, wait for rendering to finish. When false, you can manually call this function repeatedly until it stops returning MVD_STATUS_BUSY.
 */
Result mvdstdRenderVideoFrame(MVDSTD_Config* config, bool wait);

/**
 * @brief Sets the current configuration of MVDSTD.
 * @param config Pointer to the configuration to set.
 */
Result MVDSTD_SetConfig(MVDSTD_Config* config);

