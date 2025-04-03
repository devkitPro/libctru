/**
 * @file mvd.h
 * @brief MVD service.
 */
#pragma once

//New3DS-only, see also: http://3dbrew.org/wiki/MVD_Services

///These values are the data returned as "result-codes" by MVDSTD.
#define MVD_STATUS_OK 0x17000
#define MVD_STATUS_PARAMSET 0x17001 ///"Returned after processing NAL-unit parameter-sets."
#define MVD_STATUS_BUSY 0x17002
#define MVD_STATUS_FRAMEREADY 0x17003
#define MVD_STATUS_INCOMPLETEPROCESSING 0x17004 ///"Returned when not all of the input NAL-unit buffer was processed."
#define MVD_STATUS_NALUPROCFLAG 0x17007 ///See here: https://www.3dbrew.org/wiki/MVDSTD:ProcessNALUnit

///This can be used to check whether mvdstdProcessVideoFrame() was successful.
#define MVD_CHECKNALUPROC_SUCCESS(x) (x==MVD_STATUS_OK || x==MVD_STATUS_PARAMSET || x==MVD_STATUS_FRAMEREADY || x==MVD_STATUS_INCOMPLETEPROCESSING || x==MVD_STATUS_NALUPROCFLAG)

/// Default input size for mvdstdInit(). This is what the New3DS Internet Browser uses, from the MVDSTD:CalculateWorkBufSize output.
#define MVD_DEFAULT_WORKBUF_SIZE 0x9006C8

#define MVD_CALC_WITH_LEVEL_FLAG_NONE				(u8)(0x00)	//Nothing.
#define MVD_CALC_WITH_LEVEL_FLAG_ENABLE_CALC		(u8)(0x01)	//Enable calculation with level.
#define MVD_CALC_WITH_LEVEL_FLAG_ENABLE_EXTRA_OP	(u8)(0x02)	//Enable extra op after base calculation (see : https://www.3dbrew.org/wiki/MVDSTD:CalculateWorkBufSize).
#define MVD_CALC_WITH_LEVEL_FLAG_UNK				(u8)(0x04)	//Unknown.

#define MVD_H264_LEVEL_1_0		(u8)(0x00)	//H.264 level 1.0.
#define MVD_H264_LEVEL_1_0B		(u8)(0x01)	//H.264 level 1.0b.
#define MVD_H264_LEVEL_1_1		(u8)(0x02)	//H.264 level 1.1.
#define MVD_H264_LEVEL_1_2		(u8)(0x03)	//H.264 level 1.2.
#define MVD_H264_LEVEL_1_3		(u8)(0x04)	//H.264 level 1.3.
#define MVD_H264_LEVEL_2_0		(u8)(0x05)	//H.264 level 2.0.
#define MVD_H264_LEVEL_2_1		(u8)(0x06)	//H.264 level 2.1.
#define MVD_H264_LEVEL_2_2		(u8)(0x07)	//H.264 level 2.2.
#define MVD_H264_LEVEL_3_0		(u8)(0x08)	//H.264 level 3.0.
#define MVD_H264_LEVEL_3_1		(u8)(0x09)	//H.264 level 3.1.
#define MVD_H264_LEVEL_3_2		(u8)(0x0A)	//H.264 level 3.2.
#define MVD_H264_LEVEL_4_0		(u8)(0x0B)	//H.264 level 4.0.
#define MVD_H264_LEVEL_4_1		(u8)(0x0C)	//H.264 level 4.1.
#define MVD_H264_LEVEL_4_2		(u8)(0x0D)	//H.264 level 4.2.
#define MVD_H264_LEVEL_5_0		(u8)(0x0E)	//H.264 level 5.0.
#define MVD_H264_LEVEL_5_1		(u8)(0x0F)	//H.264 level 5.1.
#define MVD_H264_LEVEL_5_2		(u8)(0x10)	//H.264 level 5.2.

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
	MVD_OUTPUT_YUYV422 = 0x00010001, ///< YUYV422
	MVD_OUTPUT_BGR565 = 0x00040002,  ///< BGR565
	MVD_OUTPUT_RGB565 = 0x00040004   ///< RGB565
} MVDSTD_OutputFormat;

/// Processing configuration.
typedef struct {
	MVDSTD_InputFormat input_type;   ///< Input type.
	u32 unk_x04;                     ///< Unknown.
	u32 unk_x08;                     ///< Unknown. Referred to as "H264 range" in SKATER.
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
	MVDSTD_OutputFormat output_type; ///< Output type.
	u32 outwidth;                    ///< Output width.
	u32 outheight;                   ///< Output height.
	u32 physaddr_outdata0;           ///< Physical address of output data.
	u32 physaddr_outdata1;           ///< Additional physical address for output data, only used when the output format type is value 0x00020001.
	u32 unk_x6c[0x98>>2];            ///< Unknown.
	u32 flag_x104;                   ///< This enables using the following 4 words when non-zero.
	u32 output_x_pos;                ///< Output X position in the output buffer.
	u32 output_y_pos;                ///< Same as above except for the Y pos.
	u32 output_width_override;       ///< Used for aligning the output width when larger than the output width. Overrides the output width when smaller than the output width.
	u32 output_height_override;      ///< Same as output_width_override except for the output height.
	u32 unk_x118;
} MVDSTD_Config;

typedef struct {
	u32 end_vaddr;//"End-address of the processed NAL-unit(internal MVD heap vaddr)."
	u32 end_physaddr;//"End-address of the processed NAL-unit(physaddr following the input physaddr)."
	u32 remaining_size;//"Total remaining unprocessed input data. Buffer_end_pos=bufsize-<this value>."
} MVDSTD_ProcessNALUnitOut;

typedef struct {
	void* outdata0;//Linearmem vaddr equivalent to config *_outdata0.
	void* outdata1;//Linearmem vaddr equivalent to config *_outdata1.
} MVDSTD_OutputBuffersEntry;

typedef struct {
	u32 total_entries;//Total actual used entries below.
	MVDSTD_OutputBuffersEntry entries[17];
} MVDSTD_OutputBuffersEntryList;

/// This can be used to override the default input values for MVDSTD commands during initialization with video-processing. The default for these fields are all-zero, except for cmd1b_inval which is 1. See also here: https://www.3dbrew.org/wiki/MVD_Services
typedef struct {
	s8 cmd5_inval0, cmd5_inval1, cmd5_inval2;
	u32 cmd5_inval3;

	u8 cmd1b_inval;
} MVDSTD_InitStruct;

typedef struct
{
	u8 enable;			//Whether use this calculation method.
	u8 flag;			//Flag for calculation (MVD_CALC_WITH_LEVEL_FLAG_XXXXXX).
	u8 double_size;		//If set, size calculation result is doubled.
	u8 level;			//H.264 (AVC) level (MVD_H264_LEVEL_XXXXXX).
} MVDSTD_WithLevel;

typedef struct
{
	u8 enable;			//Whether use this calculation method.
	u8 ref_frames;		//Number of reference frames.
} MVDSTD_WithNumOfRefFrames;

/// H.264 buffer calculation configuration.
/// See here for detailed explanations : https://www.3dbrew.org/wiki/MVDSTD:CalculateWorkBufSize.
typedef struct {
	u8 unused_0x00;								//Unused.
	MVDSTD_WithLevel level;						//Calc buffer size with H.264 level.
	MVDSTD_WithNumOfRefFrames ref_frames_a;		//Calc buffer size with num of reference frames and resolution.
	MVDSTD_WithNumOfRefFrames ref_frames_b;		//Calc buffer size with num of reference frames and resolution.
	u8 unused_0x09[3];							//Unused.
	u32 unk_0x0c;								//Unknown.
	u32 unk_0x10;								//Unknown.
	u32 unk_0x14;								//Unknown.
	u32 unk_0x18;								//Unknown.
	u32 unk_0x1c;								//Unknown.
	u32 unk_0x20;								//Unknown.
	u32 unk_0x24;								//Unknown.
	u32 width;									//Video width.
	u32 height;									//Video height.
} MVDSTD_CalculateWorkBufSizeConfig;

/**
 * @brief Initializes MVDSTD.
 * @param mode Mode to initialize MVDSTD to.
 * @param input_type Type of input to process.
 * @param output_type Type of output to produce.
 * @param size Size of the work buffer, MVD_DEFAULT_WORKBUF_SIZE can be used for this. Only used when type == MVDMODE_VIDEOPROCESSING.
 * @param initstruct Optional MVDSTD_InitStruct, this should be NULL normally.
 */
Result mvdstdInit(MVDSTD_Mode mode, MVDSTD_InputFormat input_type, MVDSTD_OutputFormat output_type, u32 size, MVDSTD_InitStruct *initstruct);

/// Shuts down MVDSTD.
void mvdstdExit(void);

/**
 * @brief Calculate working buffer size for H.264 decoding.
 * @param config Calculation config, see here for detailed explanations : https://www.3dbrew.org/wiki/MVDSTD:CalculateWorkBufSize.
 * @param size_out Calculated buffer size in bytes.
 */
Result mvdstdCalculateBufferSize(MVDSTD_CalculateWorkBufSizeConfig* config, u32* size_out);

/**
 * @brief Generates a default MVDSTD configuration.
 * @param config Pointer to output the generated config to.
 * @param input_width Input width.
 * @param input_height Input height.
 * @param output_width Output width.
 * @param output_height Output height.
 * @param vaddr_colorconv_indata Virtual address of the color conversion input data.
 * @param vaddr_outdata0 Virtual address of the output data.
 * @param vaddr_outdata1 Additional virtual address for output data, only used when the output format type is value 0x00020001.
 */
void mvdstdGenerateDefaultConfig(MVDSTD_Config*config, u32 input_width, u32 input_height, u32 output_width, u32 output_height, u32 *vaddr_colorconv_indata, u32 *vaddr_outdata0, u32 *vaddr_outdata1);

/**
 * @brief Run color-format-conversion.
 * @param config Pointer to the configuration to use.
 */
Result mvdstdConvertImage(MVDSTD_Config* config);

/**
 * @brief Processes a video frame(specifically a NAL-unit).
 * @param inbuf_vaddr Input NAL-unit starting with the 3-byte "00 00 01" prefix. Must be located in linearmem.
 * @param size Size of the input buffer.
 * @param flag See here regarding this input flag: https://www.3dbrew.org/wiki/MVDSTD:ProcessNALUnit
 * @param out Optional output MVDSTD_ProcessNALUnitOut structure.
 */
Result mvdstdProcessVideoFrame(void* inbuf_vaddr, size_t size, u32 flag, MVDSTD_ProcessNALUnitOut *out);

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

/**
 * @brief New3DS Internet Browser doesn't use this. Once done, rendered frames will be written to the output buffers specified by the entrylist instead of the output specified by configuration. See here: https://www.3dbrew.org/wiki/MVDSTD:SetupOutputBuffers
 * @param entrylist Input entrylist.
 * @param bufsize Size of each buffer from the entrylist.
 */
Result mvdstdSetupOutputBuffers(MVDSTD_OutputBuffersEntryList *entrylist, u32 bufsize);

/**
 * @brief New3DS Internet Browser doesn't use this. This overrides the entry0 output buffers originally setup by mvdstdSetupOutputBuffers(). See also here: https://www.3dbrew.org/wiki/MVDSTD:OverrideOutputBuffers
 * @param cur_outdata0 Linearmem vaddr. The current outdata0 for this entry must match this value.
 * @param cur_outdata1 Linearmem vaddr. The current outdata1 for this entry must match this value.
 * @param new_outdata0 Linearmem vaddr. This is the new address to use for outaddr0.
 * @param new_outdata1 Linearmem vaddr. This is the new address to use for outaddr1.
 */
Result mvdstdOverrideOutputBuffers(void* cur_outdata0, void* cur_outdata1, void* new_outdata0, void* new_outdata1);

