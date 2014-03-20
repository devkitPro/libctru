#ifndef GPU_H
#define GPU_H

void GPU_Init(Handle *gsphandle);
void GPU_Reset(u32* gxbuf, u32* gpuBuf, u32 gpuBufSize);

void GPUCMD_SetBuffer(u32* adr, u32 size, u32 offset);
void GPUCMD_Run(u32* gxbuf);
void GPUCMD_Add(u32 cmd, u32* param, u32 paramlength);
void GPUCMD_AddSingleParam(u32 cmd, u32 param);
void GPUCMD_Finalize();

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

void GPU_SetUniform(u32 startreg, u32* data, u32 numreg);
void GPU_SetViewport(u32* depthBuffer, u32* colorBuffer, u32 x, u32 y, u32 w, u32 h);
void GPU_DepthRange(float nearVal, float farVal);
void GPU_SetDepthTest(bool enable, GPU_TESTFUNC function, u8 ref);
void GPU_SetStencilTest(bool enable, GPU_TESTFUNC function, u8 ref);
void GPU_SetFaceCulling(GPU_CULLMODE mode);
void GPU_SetAttributeBuffers(u8 totalAttributes, u32* baseAddress, u64 attributeFormats, u16 attributeMask, u64 attributePermutation, u8 numBuffers, u32 bufferOffsets[], u64 bufferPermutations[], u8 bufferNumAttributes[]);
void GPU_SetTexture(u32* data, u16 width, u16 height, u32 param, GPU_TEXCOLOR colorType);
void GPU_SetTexEnv(u8 id, u16 rgbSources, u16 alphaSources, u16 rgbOperands, u16 alphaOperands, GPU_COMBINEFUNC rgbCombine, GPU_COMBINEFUNC alphaCombine, u32 constantColor);

#endif
