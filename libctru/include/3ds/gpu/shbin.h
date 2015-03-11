#pragma once

#include <3ds/gpu/gpu.h>

typedef enum{
	VERTEX_SHDR=GPU_VERTEX_SHADER,
	GEOMETRY_SHDR=GPU_GEOMETRY_SHADER
}DVLE_type;

typedef enum{
	DVLE_CONST_BOOL=0x0,
	DVLE_CONST_u8=0x1,
	DVLE_CONST_FLOAT24=0x2,
}DVLE_constantType;

typedef enum{
	RESULT_POSITION = 0x0,
	RESULT_NORMALQUAT = 0x1,
	RESULT_COLOR = 0x2,
	RESULT_TEXCOORD0 = 0x3,
	RESULT_TEXCOORD0W = 0x4,
	RESULT_TEXCOORD1 = 0x5,
	RESULT_TEXCOORD2 = 0x6,
	RESULT_VIEW = 0x8
}DVLE_outputAttribute_t;

typedef struct{
	u32 codeSize;
	u32* codeData;
	u32 opdescSize;
	u32* opcdescData;
}DVLP_s;

typedef struct{
	u16 type;
	u16 id;
	u32 data[4];
}DVLE_constEntry_s;

typedef struct{
	u16 type;
	u16 regID;
	u8 mask;
	u8 unk[3];
}DVLE_outEntry_s;

typedef struct{
	u32 symbolOffset;
	u16 startReg;
	u16 endReg;
}DVLE_uniformEntry_s;

typedef struct{
	DVLE_type type;
	DVLP_s* dvlp;
	u32 mainOffset, endmainOffset;
	u32 constTableSize;
	DVLE_constEntry_s* constTableData;
	u32 outTableSize;
	DVLE_outEntry_s* outTableData;
	u32 uniformTableSize;
	DVLE_uniformEntry_s* uniformTableData;
	char* symbolTableData;
	u8 outmapMask;
	u32 outmapData[8];
}DVLE_s;

typedef struct{
	u32 numDVLE;
	DVLP_s DVLP;
	DVLE_s* DVLE;
}DVLB_s;

DVLB_s* DVLB_ParseFile(u32* shbinData, u32 shbinSize);
void DVLB_Free(DVLB_s* dvlb);

s8 DVLE_GetUniformRegister(DVLE_s* dvle, const char* name);
void DVLE_GenerateOutmap(DVLE_s* dvle);
