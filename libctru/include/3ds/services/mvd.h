#pragma once

//New3DS-only, see also: http://3dbrew.org/wiki/MVD_Services

typedef enum {
	MVDTYPE_COLORFORMATCONV = 0x00010001,
	MVDTYPE_H264 = 0x00020001
} mvdstdType;

typedef struct {
	mvdstdType type;
	u32 unk_x04;
	u32 unk_x08;
	u32 width0, height0;
	u32 physaddr_colorconv_indata;
	u32 unk_x18[0x28>>2];
	u32 flag_x40;//0x0 for colorconv, 0x1 for H.264
	u32 unk_x44;
	u32 unk_x48;
	u32 height1, width1;//Only set for H.264.
	u32 unk_x54;
	u32 unk_x58;
	u32 width2, height2;
	u32 physaddr_outdata0;
	u32 physaddr_outdata1_colorconv;
	u32 unk_x6c[0xb0>>2];
} mvdstdConfig;

void mvdstdGenerateDefaultConfig(mvdstdConfig *config, u32 width, u32 height, u32 *vaddr_colorconv_indata, u32 *vaddr_outdata0, u32 *vaddr_outdata1_colorconv);

Result mvdstdInit(mvdstdType type, u32 size);//The input size isn't used when type==MVDTYPE_COLORFORMATCONV. H.264 isn't supported currently.
Result mvdstdShutdown();

Result mvdstdSetConfig(mvdstdConfig *config);
Result mvdstdProcessFrame(mvdstdConfig *config, u32 *h264_vaddr_inframe, u32 h264_inframesize, u32 h264_frameid);

