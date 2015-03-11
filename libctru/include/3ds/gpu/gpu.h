#pragma once

#include "3ds/gpu/registers.h"

//GPU
void GPU_Init(Handle *gsphandle);
void GPU_Reset(u32* gxbuf, u32* gpuBuf, u32 gpuBufSize);

//GPUCMD
#define GPUCMD_HEADER(incremental, mask, reg) (((incremental)<<31)|(((mask)&0xF)<<16)|((reg)&0x3FF))

void GPUCMD_SetBuffer(u32* adr, u32 size, u32 offset);
void GPUCMD_SetBufferOffset(u32 offset);
void GPUCMD_GetBuffer(u32** adr, u32* size, u32* offset);
void GPUCMD_AddRawCommands(u32* cmd, u32 size);
void GPUCMD_Run(u32* gxbuf);
void GPUCMD_FlushAndRun(u32* gxbuf);
void GPUCMD_Add(u32 header, u32* param, u32 paramlength);
void GPUCMD_Finalize();

#define GPUCMD_AddSingleParam(header, param) GPUCMD_Add((header), (u32[]){(u32)(param)}, 1)

#define GPUCMD_AddMaskedWrite(reg, mask, val) GPUCMD_AddSingleParam(GPUCMD_HEADER(0, (mask), (reg)), (val))
#define GPUCMD_AddWrite(reg, val) GPUCMD_AddMaskedWrite((reg), 0xF, (val))
#define GPUCMD_AddMaskedWrites(reg, mask, vals, num) GPUCMD_Add(GPUCMD_HEADER(0, (mask), (reg)), (vals), (num))
#define GPUCMD_AddWrites(reg, vals, num) GPUCMD_AddMaskedWrites((reg), 0xF, (vals), (num))
#define GPUCMD_AddMaskedIncrementalWrites(reg, mask, vals, num) GPUCMD_Add(GPUCMD_HEADER(1, (mask), (reg)), (vals), (num))
#define GPUCMD_AddIncrementalWrites(reg, vals, num) GPUCMD_AddMaskedIncrementalWrites((reg), 0xF, (vals), (num))

//tex param
#define GPU_TEXTURE_MAG_FILTER(v) (((v)&0x1)<<1) //takes a GPU_TEXTURE_FILTER_PARAM
#define GPU_TEXTURE_MIN_FILTER(v) (((v)&0x1)<<2) //takes a GPU_TEXTURE_FILTER_PARAM
#define GPU_TEXTURE_WRAP_S(v) (((v)&0x3)<<8) //takes a GPU_TEXTURE_WRAP_PARAM
#define GPU_TEXTURE_WRAP_T(v) (((v)&0x3)<<12) //takes a GPU_TEXTURE_WRAP_PARAM

typedef enum
{
	GPU_NEAREST = 0x0,
	GPU_LINEAR = 0x1,
}GPU_TEXTURE_FILTER_PARAM;

typedef enum
{
	GPU_CLAMP_TO_EDGE = 0x0,
	GPU_REPEAT = 0x2,
}GPU_TEXTURE_WRAP_PARAM;

typedef enum
{
	GPU_TEXUNIT0 = 0x1,
	GPU_TEXUNIT1 = 0x2,
	GPU_TEXUNIT2 = 0x4
} GPU_TEXUNIT;

typedef enum{
	GPU_RGBA8=0x0,
	GPU_RGB8=0x1,
	GPU_RGBA5551=0x2,
	GPU_RGB565=0x3,
	GPU_RGBA4=0x4,
	GPU_LA8=0x5,
	GPU_HILO8=0x6,
	GPU_L8=0x7,
	GPU_A8=0x8,
	GPU_LA4=0x9,
	GPU_L4=0xA,
	GPU_ETC1=0xB,
	GPU_ETC1A4=0xC
}GPU_TEXCOLOR;

typedef enum
{
	GPU_NEVER = 0,
	GPU_ALWAYS = 1,
	GPU_EQUAL = 2,
	GPU_NOTEQUAL = 3,
	GPU_LESS = 4,
	GPU_LEQUAL = 5,
	GPU_GREATER = 6,
	GPU_GEQUAL = 7
}GPU_TESTFUNC;

typedef enum
{
	GPU_SCISSOR_DISABLE = 0,	// disable scissor test
	GPU_SCISSOR_INVERT = 1,		// exclude pixels inside the scissor box
	// 2 is the same as 0
	GPU_SCISSOR_NORMAL = 3,		// exclude pixels outside of the scissor box
	
} GPU_SCISSORMODE;

typedef enum
{
	GPU_KEEP = 0, 		// keep destination value
	GPU_AND_NOT = 1, 	// destination & ~source
	GPU_XOR = 5,		// destination ^ source
	// 2 is the same as 1. Other values are too weird to even be usable.
} GPU_STENCILOP;

typedef enum
{
	GPU_WRITE_RED = 0x01,
	GPU_WRITE_GREEN = 0x02,
	GPU_WRITE_BLUE = 0x04,
	GPU_WRITE_ALPHA = 0x08,
	GPU_WRITE_DEPTH = 0x10,
	
	GPU_WRITE_COLOR = 0x0F,
	GPU_WRITE_ALL = 0x1F
} GPU_WRITEMASK;

typedef enum
{
	GPU_BLEND_ADD = 0,
	GPU_BLEND_SUBTRACT = 1,
	GPU_BLEND_REVERSE_SUBTRACT = 2,
	GPU_BLEND_MIN = 3,
	GPU_BLEND_MAX = 4
} GPU_BLENDEQUATION;

typedef enum
{
	GPU_ZERO = 0,
	GPU_ONE = 1,
	GPU_SRC_COLOR = 2,
	GPU_ONE_MINUS_SRC_COLOR = 3,
	GPU_DST_COLOR = 4,
	GPU_ONE_MINUS_DST_COLOR = 5,
	GPU_SRC_ALPHA = 6,
	GPU_ONE_MINUS_SRC_ALPHA = 7,
	GPU_DST_ALPHA = 8,
	GPU_ONE_MINUS_DST_ALPHA = 9,
	GPU_CONSTANT_COLOR = 10,
	GPU_ONE_MINUS_CONSTANT_COLOR = 11,
	GPU_CONSTANT_ALPHA = 12,
	GPU_ONE_MINUS_CONSTANT_ALPHA = 13,
	GPU_SRC_ALPHA_SATURATE = 14
} GPU_BLENDFACTOR;

typedef enum
{
	GPU_LOGICOP_CLEAR = 0,
	GPU_LOGICOP_AND = 1,
	GPU_LOGICOP_AND_REVERSE = 2,
	GPU_LOGICOP_COPY = 3,
	GPU_LOGICOP_SET = 4,
	GPU_LOGICOP_COPY_INVERTED = 5,
	GPU_LOGICOP_NOOP = 6,
	GPU_LOGICOP_INVERT = 7,
	GPU_LOGICOP_NAND = 8,
	GPU_LOGICOP_OR = 9,
	GPU_LOGICOP_NOR = 10,
	GPU_LOGICOP_XOR = 11,
	GPU_LOGICOP_EQUIV = 12,
	GPU_LOGICOP_AND_INVERTED = 13,
	GPU_LOGICOP_OR_REVERSE = 14,
	GPU_LOGICOP_OR_INVERTED = 15
} GPU_LOGICOP;

typedef enum{
	GPU_BYTE = 0,
	GPU_UNSIGNED_BYTE = 1,
	GPU_SHORT = 2,
	GPU_FLOAT = 3
}GPU_FORMATS;

//defines for CW ?
typedef enum{
	GPU_CULL_NONE = 0,
	GPU_CULL_FRONT_CCW = 1,
	GPU_CULL_BACK_CCW = 2
}GPU_CULLMODE;

#define GPU_ATTRIBFMT(i, n, f) (((((n)-1)<<2)|((f)&3))<<((i)*4))

typedef enum{
	GPU_PRIMARY_COLOR = 0x00,
	GPU_TEXTURE0 = 0x03,
	GPU_TEXTURE1 = 0x04,
	GPU_TEXTURE2 = 0x05,
	GPU_TEXTURE3 = 0x06,
	GPU_CONSTANT = 0x0E,
	GPU_PREVIOUS = 0x0F,
}GPU_TEVSRC;

typedef enum{
	GPU_REPLACE = 0x00,
	GPU_MODULATE = 0x01,
	GPU_ADD = 0x02,
	GPU_ADD_SIGNED = 0x03,
	GPU_INTERPOLATE = 0x04,
	GPU_SUBTRACT = 0x05,
	GPU_DOT3_RGB = 0x06 //RGB only
}GPU_COMBINEFUNC;

#define GPU_TEVSOURCES(a,b,c) (((a))|((b)<<4)|((c)<<8))
#define GPU_TEVOPERANDS(a,b,c) (((a))|((b)<<4)|((c)<<8))

typedef enum{
	GPU_TRIANGLES = 0x0000,
	GPU_TRIANGLE_STRIP = 0x0100,
	GPU_TRIANGLE_FAN = 0x0200,
	GPU_UNKPRIM = 0x0300 // ?
}GPU_Primitive_t;

typedef enum{
	GPU_VERTEX_SHADER=0x0,
	GPU_GEOMETRY_SHADER=0x1
}GPU_SHADER_TYPE;

void GPU_SetFloatUniform(GPU_SHADER_TYPE type, u32 startreg, u32* data, u32 numreg);

void GPU_SetViewport(u32* depthBuffer, u32* colorBuffer, u32 x, u32 y, u32 w, u32 h);

void GPU_SetScissorTest(GPU_SCISSORMODE mode, u32 x, u32 y, u32 w, u32 h);

void GPU_DepthMap(float zScale, float zOffset);
void GPU_SetAlphaTest(bool enable, GPU_TESTFUNC function, u8 ref);
void GPU_SetDepthTestAndWriteMask(bool enable, GPU_TESTFUNC function, GPU_WRITEMASK writemask); // GPU_WRITEMASK values can be ORed together
void GPU_SetStencilTest(bool enable, GPU_TESTFUNC function, u8 ref, u8 mask, u8 replace);
void GPU_SetStencilOp(GPU_STENCILOP sfail, GPU_STENCILOP dfail, GPU_STENCILOP pass);
void GPU_SetFaceCulling(GPU_CULLMODE mode);

// these two can't be used together
void GPU_SetAlphaBlending(GPU_BLENDEQUATION colorEquation, GPU_BLENDEQUATION alphaEquation, 
	GPU_BLENDFACTOR colorSrc, GPU_BLENDFACTOR colorDst, 
	GPU_BLENDFACTOR alphaSrc, GPU_BLENDFACTOR alphaDst);
void GPU_SetColorLogicOp(GPU_LOGICOP op);

void GPU_SetBlendingColor(u8 r, u8 g, u8 b, u8 a);

void GPU_SetAttributeBuffers(u8 totalAttributes, u32* baseAddress, u64 attributeFormats, u16 attributeMask, u64 attributePermutation, u8 numBuffers, u32 bufferOffsets[], u64 bufferPermutations[], u8 bufferNumAttributes[]);

void GPU_SetTextureEnable(GPU_TEXUNIT units); // GPU_TEXUNITx values can be ORed together to enable multiple texture units
void GPU_SetTexture(GPU_TEXUNIT unit, u32* data, u16 width, u16 height, u32 param, GPU_TEXCOLOR colorType);
void GPU_SetTexEnv(u8 id, u16 rgbSources, u16 alphaSources, u16 rgbOperands, u16 alphaOperands, GPU_COMBINEFUNC rgbCombine, GPU_COMBINEFUNC alphaCombine, u32 constantColor);

void GPU_DrawArray(GPU_Primitive_t primitive, u32 n);
void GPU_DrawElements(GPU_Primitive_t primitive, u32* indexArray, u32 n);
void GPU_FinishDrawing();

void GPU_SetShaderOutmap(u32 outmapData[8]);
void GPU_SendShaderCode(GPU_SHADER_TYPE type, u32* data, u16 offset, u16 length);
void GPU_SendOperandDescriptors(GPU_SHADER_TYPE type, u32* data, u16 offset, u16 length);
