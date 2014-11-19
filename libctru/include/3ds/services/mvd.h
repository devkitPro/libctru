#pragma once

//New3DS-only, see also: http://3dbrew.org/wiki/MVD_Services

typedef enum {
	MVDMODE_COLORFORMATCONV,
	MVDMODE_VIDEOPROCESSING
} mvdstdMode;

typedef enum {
	MVDTYPEIN_YUYV422 = 0x00010001,
	MVDTYPEIN_H264 = 0x00020001
} mvdstdTypeInput;

typedef enum {
	MVDTYPEOUT_RGB565 = 0x00040002
} mvdstdTypeOutput;

typedef struct {
	mvdstdTypeInput input_type;
	u32 unk_x04;
	u32 unk_x08;
	u32 inwidth, inheight;
	u32 physaddr_colorconv_indata;
	u32 unk_x18[0x28>>2];
	u32 flag_x40;//0x0 for colorconv, 0x1 for H.264
	u32 unk_x44;
	u32 unk_x48;
	u32 outheight0, outwidth0;//Only set for H.264.
	u32 unk_x54;
	mvdstdTypeOutput output_type;
	u32 outwidth1, outheight1;
	u32 physaddr_outdata0;
	u32 physaddr_outdata1_colorconv;
	u32 unk_x6c[0xb0>>2];
} mvdstdConfig;

void mvdstdGenerateDefaultConfig(mvdstdConfig *config, u32 input_width, u32 input_height, u32 output_width, u32 output_height, u32 *vaddr_colorconv_indata, u32 *vaddr_outdata0, u32 *vaddr_outdata1_colorconv);

Result mvdstdInit(mvdstdMode mode, mvdstdTypeInput input_type, mvdstdTypeOutput output_type, u32 size);//The input size isn't used when type==MVDTYPE_COLORFORMATCONV. Video processing / H.264 isn't supported currently.
Result mvdstdShutdown();

Result mvdstdSetConfig(mvdstdConfig *config);
Result mvdstdProcessFrame(mvdstdConfig *config, u32 *h264_vaddr_inframe, u32 h264_inframesize, u32 h264_frameid);

