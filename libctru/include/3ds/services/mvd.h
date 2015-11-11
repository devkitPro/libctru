/**
 * @file mvd.h
 * @brief MVD service.
 */
#pragma once

//New3DS-only, see also: http://3dbrew.org/wiki/MVD_Services

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
	MVD_OUTPUT_RGB565 = 0x00040002 ///< RGB565
} MVDSTD_OutputFormat;

/// Processing configuration.
typedef struct {
	MVDSTD_InputFormat input_type;      ///< Input type.
	u32 unk_x04;                     ///< Unknown.
	u32 unk_x08;                     ///< Unknown.
	u32 inwidth;                     ///< Input width.
	u32 inheight;                    ///< Input height.
	u32 physaddr_colorconv_indata;   ///< Physical address of color conversion input data.
	u32 unk_x18[0x28>>2];            ///< Unknown.
	u32 flag_x40;                    ///< Unknown. 0x0 for colorconv, 0x1 for H.264
	u32 unk_x44;                     ///< Unknown.
	u32 unk_x48;                     ///< Unknown.
	u32 outheight0;                  ///< First output width. Only set for H.264.
	u32 outwidth0;                   ///< First output height. Only set for H.264.
	u32 unk_x54;                     ///< Unknown.
	MVDSTD_OutputFormat output_type;    ///< Output type.
	u32 outwidth1;                   ///< Second output width.
	u32 outheight1;                  ///< Second output height.
	u32 physaddr_outdata0;           ///< Physical address of output data.
	u32 physaddr_outdata1_colorconv; ///< Physical address of color conversion output data.
	u32 unk_x6c[0xb0>>2];            ///< Unknown.
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
 * @brief Processes a frame.
 * @param config Pointer to the configuration to use.
 * @param h264_vaddr_inframe Input H264 frame.
 * @param h264_inframesize Size of the input frame.
 * @param h264_frameid ID of the input frame.
 */
Result mvdstdProcessFrame(MVDSTD_Config*config, u32 *h264_vaddr_inframe, u32 h264_inframesize, u32 h264_frameid);

/**
 * @brief Sets the current configuration of MVDSTD.
 * @param config Pointer to the configuration to set.
 */
Result MVDSTD_SetConfig(MVDSTD_Config* config);

