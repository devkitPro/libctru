#pragma once

#include "gpu.h"

//GPU
void GPU_Init(Handle *gsphandle) DEPRECATED;
void GPU_Reset(u32* gxbuf, u32* gpuBuf, u32 gpuBufSize) DEPRECATED;

void GPU_SetFloatUniform(GPU_SHADER_TYPE type, u32 startreg, u32* data, u32 numreg) DEPRECATED;

void GPU_SetViewport(u32* depthBuffer, u32* colorBuffer, u32 x, u32 y, u32 w, u32 h) DEPRECATED;

void GPU_SetScissorTest(GPU_SCISSORMODE mode, u32 x, u32 y, u32 w, u32 h) DEPRECATED;

void GPU_DepthMap(float zScale, float zOffset) DEPRECATED;
void GPU_SetAlphaTest(bool enable, GPU_TESTFUNC function, u8 ref) DEPRECATED;
void GPU_SetDepthTestAndWriteMask(bool enable, GPU_TESTFUNC function, GPU_WRITEMASK writemask) DEPRECATED; // GPU_WRITEMASK values can be ORed together
void GPU_SetStencilTest(bool enable, GPU_TESTFUNC function, u8 ref, u8 input_mask, u8 write_mask) DEPRECATED;
void GPU_SetStencilOp(GPU_STENCILOP sfail, GPU_STENCILOP dfail, GPU_STENCILOP pass) DEPRECATED;
void GPU_SetFaceCulling(GPU_CULLMODE mode) DEPRECATED;
// Only the first four tev stages can write to the combiner buffer, use GPU_TEV_BUFFER_WRITE_CONFIG to build the parameters
void GPU_SetCombinerBufferWrite(u8 rgb_config, u8 alpha_config) DEPRECATED;

// these two can't be used together
void GPU_SetAlphaBlending(GPU_BLENDEQUATION colorEquation, GPU_BLENDEQUATION alphaEquation,
	GPU_BLENDFACTOR colorSrc, GPU_BLENDFACTOR colorDst,
	GPU_BLENDFACTOR alphaSrc, GPU_BLENDFACTOR alphaDst) DEPRECATED;
void GPU_SetColorLogicOp(GPU_LOGICOP op) DEPRECATED;

void GPU_SetBlendingColor(u8 r, u8 g, u8 b, u8 a) DEPRECATED;

void GPU_SetAttributeBuffers(u8 totalAttributes, u32* baseAddress, u64 attributeFormats, u16 attributeMask, u64 attributePermutation, u8 numBuffers, u32 bufferOffsets[], u64 bufferPermutations[], u8 bufferNumAttributes[]) DEPRECATED;

void GPU_SetTextureEnable(GPU_TEXUNIT units) DEPRECATED; // GPU_TEXUNITx values can be ORed together to enable multiple texture units


void GPU_SetTexture(GPU_TEXUNIT unit, u32* data, u16 width, u16 height, u32 param, GPU_TEXCOLOR colorType) DEPRECATED;

/**
 * @param borderColor The color used for the border when using the @ref GPU_CLAMP_TO_BORDER wrap mode
 */
void GPU_SetTextureBorderColor(GPU_TEXUNIT unit,u32 borderColor) DEPRECATED;
void GPU_SetTexEnv(u8 id, u16 rgbSources, u16 alphaSources, u16 rgbOperands, u16 alphaOperands, GPU_COMBINEFUNC rgbCombine, GPU_COMBINEFUNC alphaCombine, u32 constantColor) DEPRECATED;

void GPU_DrawArray(GPU_Primitive_t primitive, u32 first, u32 count) DEPRECATED;
void GPU_DrawElements(GPU_Primitive_t primitive, u32* indexArray, u32 n) DEPRECATED;
void GPU_FinishDrawing() DEPRECATED;
