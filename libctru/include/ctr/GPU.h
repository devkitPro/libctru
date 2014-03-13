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

void GPU_SetUniform(u32 startreg, u32* data, u32 numreg);
void GPU_SetViewport(u32* depthBuffer, u32* colorBuffer, u32 x, u32 y, u32 w, u32 h);
void GPU_DepthRange(float nearVal, float farVal);
void GPU_SetTexture(u32* data, u16 width, u16 height, u32 param, GPU_TEXCOLOR colorType);

#endif
