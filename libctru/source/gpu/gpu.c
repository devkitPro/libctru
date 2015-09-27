/*
  gpu.c _ Advanced GPU commands.
*/

#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/gpu/gpu.h>
#include <3ds/gpu/gx.h>
#include <3ds/gpu/shbin.h>

u32* gpuCmdBuf;
u32 gpuCmdBufSize;
u32 gpuCmdBufOffset;

void GPU_Init(Handle *gsphandle)
{
	gpuCmdBuf=NULL;
	gpuCmdBufSize=0;
	gpuCmdBufOffset=0;
}

void GPUCMD_SetBuffer(u32* adr, u32 size, u32 offset)
{
	gpuCmdBuf=adr;
	gpuCmdBufSize=size;
	gpuCmdBufOffset=offset;
}

void GPUCMD_SetBufferOffset(u32 offset)
{
	gpuCmdBufOffset=offset;
}

void GPUCMD_GetBuffer(u32** adr, u32* size, u32* offset)
{
	if(adr)*adr=gpuCmdBuf;
	if(size)*size=gpuCmdBufSize;
	if(offset)*offset=gpuCmdBufOffset;
}

void GPUCMD_AddRawCommands(u32* cmd, u32 size)
{
	if(!cmd || !size)return;

	memcpy(&gpuCmdBuf[gpuCmdBufOffset], cmd, size*4);
	gpuCmdBufOffset+=size;
}

void GPUCMD_Run(u32* gxbuf)
{
	GX_SetCommandList_First(gxbuf, gpuCmdBuf, gpuCmdBufOffset*4, NULL, 0, NULL, 0);
	GX_SetCommandList_Last(gxbuf, gpuCmdBuf, gpuCmdBufOffset*4, 0x0);
}

extern u32 __linear_heap_size;
extern u32 __linear_heap;

void GPUCMD_FlushAndRun(u32* gxbuf)
{
	//take advantage of GX_SetCommandList_First to flush gsp heap
	GX_SetCommandList_First(gxbuf, gpuCmdBuf, gpuCmdBufOffset*4, (u32 *) __linear_heap, __linear_heap_size, NULL, 0);
	GX_SetCommandList_Last(gxbuf, gpuCmdBuf, gpuCmdBufOffset*4, 0x0);
}

void GPUCMD_Add(u32 header, u32* param, u32 paramlength)
{
	u32 zero=0x0;

	if(!param || !paramlength)
	{
		paramlength=1;
		param=&zero;
	}

	if(!gpuCmdBuf || gpuCmdBufOffset+paramlength+1>gpuCmdBufSize)return;

	paramlength--;
	header|=(paramlength&0x7ff)<<20;

	gpuCmdBuf[gpuCmdBufOffset]=param[0];
	gpuCmdBuf[gpuCmdBufOffset+1]=header;

	if(paramlength)memcpy(&gpuCmdBuf[gpuCmdBufOffset+2], &param[1], paramlength*4);

	gpuCmdBufOffset+=paramlength+2;

	if(paramlength&1)gpuCmdBuf[gpuCmdBufOffset++]=0x00000000; //alignment
}

void GPUCMD_Finalize()
{
	GPUCMD_AddMaskedWrite(GPUREG_PRIMITIVE_CONFIG, 0x8, 0x00000000);
	GPUCMD_AddWrite(GPUREG_0111, 0x00000001);
	GPUCMD_AddWrite(GPUREG_0110, 0x00000001);
	GPUCMD_AddWrite(GPUREG_FINALIZE, 0x12345678);
	GPUCMD_AddWrite(GPUREG_FINALIZE, 0x12345678); //not the cleanest way of guaranteeing 0x10-byte size but whatever good enough for now
}

extern u32 gpuResetSequence[];
extern u32 gpuResetSequenceLength;

void GPU_Reset(u32* gxbuf, u32* gpuBuf, u32 gpuBufSize)
{
	int i;
	static u32 param[0x80];
	static u32 zero[0x80];
	memset(zero,  0x00, 0x80*4);

	GPUCMD_SetBuffer(gpuBuf, gpuBufSize, 0);

	GPUCMD_AddMaskedWrite(GPUREG_TEXUNITS_CONFIG, 0xD, 0x00011000);

	for(i=0x1;i<0xC;i++)GPUCMD_AddWrite(GPUREG_TEXUNITS_CONFIG+i, 0x00000000);
	GPUCMD_AddWrite(GPUREG_008C, 0x00FF0000);
	GPUCMD_AddWrite(GPUREG_008D, 0x00000000);
	GPUCMD_AddWrite(GPUREG_TEXUNIT0_TYPE, 0x00000000);

	for(i=0x0;i<0xF;i++)GPUCMD_AddWrite(GPUREG_0090+i, 0x00000000);

	GPUCMD_AddMaskedWrite(GPUREG_0245, 0x1, 0x00000001);
	GPUCMD_AddMaskedWrite(GPUREG_0244, 0x1, 0x00000000);
	GPUCMD_AddMaskedWrite(GPUREG_GSH_INPUTBUFFER_CONFIG, 0x8, 0x80000000);
	GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG, 0xB, 0x00000000);

	GPUCMD_AddWrite(GPUREG_0252, 0x00000000);
	GPUCMD_AddWrite(GPUREG_0251, 0x00000000);
	GPUCMD_AddWrite(GPUREG_0254, 0x00000000);
	GPUCMD_AddMaskedWrite(GPUREG_0253, 0x1, 0x00000000);

	GPUCMD_AddWrite(GPUREG_0242, 0x00000000);
	GPUCMD_AddWrite(GPUREG_024A, 0x00000000);

	GPUCMD_AddMaskedWrite(GPUREG_PRIMITIVE_CONFIG, 0x5, 0x00000000);

	GPUCMD_AddIncrementalWrites(GPUREG_BLEND_CONFIG, zero, 0x00000007);

	GPUCMD_AddWrite(GPUREG_011F, 0x00010140);
	GPUCMD_AddWrite(GPUREG_COLOROUTPUT_CONFIG, 0x00E40100);
	GPUCMD_AddWrite(GPUREG_BLEND_CONFIG, 0x01010000);
	GPUCMD_AddWrite(GPUREG_DEPTHTEST_CONFIG, 0x00001F40);
	GPUCMD_AddWrite(GPUREG_STENCILTEST_CONFIG, 0xFF00FF10);

	GPUCMD_AddMaskedWrite(GPUREG_0061, 0x1, 0x00000003);
	GPUCMD_AddMaskedWrite(GPUREG_0062, 0x1, 0x00000000);

	GPUCMD_AddWrite(GPUREG_SCISSORTEST_MODE, 0x00000000);
	GPUCMD_AddWrite(GPUREG_SCISSORTEST_POS, 0x00000000);
	GPUCMD_AddWrite(GPUREG_SCISSORTEST_DIM, 0x00000000);

	GPUCMD_AddMaskedWrite(GPUREG_0118, 0x1, 0x00000000);
	GPUCMD_AddWrite(GPUREG_011B, 0x00000000);

	GPUCMD_AddMaskedWrite(GPUREG_006A, 0x7, 0x00FFFFFF);

	GPUCMD_AddWrite(GPUREG_COLORLOGICOP_CONFIG, 0x00000003);

	GPUCMD_AddMaskedWrite(GPUREG_0126, 0x8, 0x03000000);

	GPUCMD_AddIncrementalWrites(GPUREG_FACECULLING_CONFIG, zero, 0x00000010);

	param[0x0]=0x1F1F1F1F;
	param[0x1]=0x1F1F1F1F;
	param[0x2]=0x1F1F1F1F;
	param[0x3]=0x1F1F1F1F;
	param[0x4]=0x1F1F1F1F;
	param[0x5]=0x1F1F1F1F;
	param[0x6]=0x1F1F1F1F;
	GPUCMD_AddIncrementalWrites(GPUREG_SH_OUTMAP_O0, param, 0x00000007);

	GPUCMD_AddWrite(GPUREG_0058, 0x00000100);
	GPUCMD_AddWrite(GPUREG_004C, 0x00000001);
	GPUCMD_AddWrite(GPUREG_006F, 0x00000000);

	GPUCMD_AddMaskedWrite(GPUREG_0060, 0x2, 0x00000000);
	GPUCMD_AddMaskedWrite(GPUREG_0069, 0xC, 0x00020000);

	GPUCMD_AddWrite(GPUREG_0113, 0x0000000F);
	GPUCMD_AddWrite(GPUREG_0112, 0x0000000F);
	GPUCMD_AddWrite(GPUREG_0114, 0x00000003);
	GPUCMD_AddWrite(GPUREG_0115, 0x00000003);

	GPUCMD_AddWrite(GPUREG_01C5, 0x00000000);
	for(i=0;i<32;i++)GPUCMD_AddIncrementalWrites(GPUREG_01C8, zero, 0x00000008);
	GPUCMD_AddWrite(GPUREG_01C5, 0x00000100);
	for(i=0;i<32;i++)GPUCMD_AddIncrementalWrites(GPUREG_01C8, zero, 0x00000008);
	GPUCMD_AddWrite(GPUREG_01C5, 0x00000200);
	for(i=0;i<32;i++)GPUCMD_AddIncrementalWrites(GPUREG_01C8, zero, 0x00000008);
	GPUCMD_AddWrite(GPUREG_01C5, 0x00000300);
	for(i=0;i<32;i++)GPUCMD_AddIncrementalWrites(GPUREG_01C8, zero, 0x00000008);
	GPUCMD_AddWrite(GPUREG_01C5, 0x00000400);
	for(i=0;i<32;i++)GPUCMD_AddIncrementalWrites(GPUREG_01C8, zero, 0x00000008);
	GPUCMD_AddWrite(GPUREG_01C5, 0x00000500);
	for(i=0;i<32;i++)GPUCMD_AddIncrementalWrites(GPUREG_01C8, zero, 0x00000008);
	GPUCMD_AddWrite(GPUREG_01C5, 0x00000600);
	for(i=0;i<32;i++)GPUCMD_AddIncrementalWrites(GPUREG_01C8, zero, 0x00000008);
		
	GPUCMD_AddWrite(GPUREG_GSH_FLOATUNIFORM_CONFIG, 0x80000000);
	for(i=0;i<48;i++)GPUCMD_AddIncrementalWrites(GPUREG_GSH_FLOATUNIFORM_DATA, zero, 0x00000008);
	GPUCMD_AddWrite(GPUREG_VSH_CODETRANSFER_CONFIG, 0x00000000);
	for(i=0;i<4;i++)GPUCMD_Add(0x000F02CC, zero, 0x00000080);
	GPUCMD_AddWrite(GPUREG_GSH_CODETRANSFER_CONFIG, 0x00000200);

	for(i=0;i<28;i++)GPUCMD_Add(0x000F029C, zero, 0x00000080);
	GPUCMD_AddWrite(GPUREG_VSH_CODETRANSFER_END, 0x00000000);

	GPUCMD_AddIncrementalWrites(GPUREG_VSH_INTUNIFORM_I0, zero, 4);

	param[0x0]=0xFFFFFFFF;
	param[0x1]=0xFFFFFFFF;
	GPUCMD_AddIncrementalWrites(GPUREG_GSH_ATTRIBUTES_PERMUTATION_LOW, param, 0x00000002);

	GPUCMD_AddIncrementalWrites(GPUREG_ATTRIBBUFFER0_CONFIG2, zero, 0x00000024);

	for(i=0;i<gpuResetSequenceLength;i++)GPUCMD_AddSingleParam(gpuResetSequence[i*2],gpuResetSequence[i*2+1]);

	GPUCMD_Finalize();
	GPUCMD_Run(gpuBuf);
	GPUCMD_SetBufferOffset(0);
}

void GPU_SetFloatUniform(GPU_SHADER_TYPE type, u32 startreg, u32* data, u32 numreg)
{
	if(!data)return;

	int regOffset=(type==GPU_GEOMETRY_SHADER)?(-0x30):(0x0);

	GPUCMD_AddWrite(GPUREG_VSH_FLOATUNIFORM_CONFIG+regOffset, 0x80000000|startreg);
	GPUCMD_AddWrites(GPUREG_VSH_FLOATUNIFORM_DATA+regOffset, data, numreg*4);
}

// f24 has:
//  - 1 sign bit
//  - 7 exponent bits
//  - 16 mantissa bits
static u32 f32tof24(float f)
{
	u32 i;
	memcpy(&i, &f, 4);

	u32 mantissa = (i << 9) >>  9;
	s32 exponent = (i << 1) >> 24;
	u32 sign     = (i << 0) >> 31;

	// Truncate mantissa
	mantissa >>= 7;

	// Re-bias exponent
	exponent = exponent - 127 + 63;
	if (exponent < 0)
	{
		// Underflow: flush to zero
		return sign << 23;
	}
	else if (exponent > 0x7F)
	{
		// Overflow: saturate to infinity
		return sign << 23 | 0x7F << 16;
	}

	return sign << 23 | exponent << 16 | mantissa;
}

// f31 has:
//  - 1 sign bit
//  - 7 exponent bits
//  - 23 mantissa bits
static u32 f32tof31(float f)
{
	u32 i;
	memcpy(&i, &f, 4);

	u32 mantissa = (i << 9) >>  9;
	s32 exponent = (i << 1) >> 24;
	u32 sign     = (i << 0) >> 31;

	// Re-bias exponent
	exponent = exponent - 127 + 63;
	if (exponent < 0)
	{
		// Underflow: flush to zero
		return sign << 30;
	}
	else if (exponent > 0x7F)
	{
		// Overflow: saturate to infinity
		return sign << 30 | 0x7F << 23;
	}

	return sign << 30 | exponent << 23 | mantissa;
}

//takes PAs as arguments
void GPU_SetViewport(u32* depthBuffer, u32* colorBuffer, u32 x, u32 y, u32 w, u32 h)
{
	u32 param[0x4];
	float fw=(float)w;
	float fh=(float)h;

	GPUCMD_AddWrite(GPUREG_0111, 0x00000001);
	GPUCMD_AddWrite(GPUREG_0110, 0x00000001);

	u32 f116e=0x01000000|(((h-1)&0xFFF)<<12)|(w&0xFFF);

	param[0x0]=((u32)depthBuffer)>>3;
	param[0x1]=((u32)colorBuffer)>>3;
	param[0x2]=f116e;
	GPUCMD_AddIncrementalWrites(GPUREG_DEPTHBUFFER_LOC, param, 0x00000003);

	GPUCMD_AddWrite(GPUREG_006E, f116e);
	GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT, 0x00000003); //depth buffer format
	GPUCMD_AddWrite(GPUREG_COLORBUFFER_FORMAT, 0x00000002); //color buffer format
	GPUCMD_AddWrite(GPUREG_011B, 0x00000000); //?

	param[0x0]=f32tof24(fw/2);
	param[0x1]=f32tof31(2.0f / fw) << 1;
	param[0x2]=f32tof24(fh/2);
	param[0x3]=f32tof31(2.0f / fh) << 1;
	GPUCMD_AddIncrementalWrites(GPUREG_0041, param, 0x00000004);

	GPUCMD_AddWrite(GPUREG_0068, (y<<16)|(x&0xFFFF));

	param[0x0]=0x00000000;
	param[0x1]=0x00000000;
	param[0x2]=((h-1)<<16)|((w-1)&0xFFFF);
	GPUCMD_AddIncrementalWrites(GPUREG_SCISSORTEST_MODE, param, 0x00000003);

	//enable depth buffer
	param[0x0]=0x0000000F;
	param[0x1]=0x0000000F;
	param[0x2]=0x00000002;
	param[0x3]=0x00000002;
	GPUCMD_AddIncrementalWrites(GPUREG_0112, param, 0x00000004);
}

void GPU_SetScissorTest(GPU_SCISSORMODE mode, u32 x, u32 y, u32 w, u32 h)
{
	u32 param[3];
	
	param[0x0] = mode;
	param[0x1] = (y<<16)|(x&0xFFFF);
	param[0x2] = ((h-1)<<16)|((w-1)&0xFFFF);
	GPUCMD_AddIncrementalWrites(GPUREG_SCISSORTEST_MODE, param, 0x00000003);
}

void GPU_DepthMap(float zScale, float zOffset)
{
	GPUCMD_AddWrite(GPUREG_006D, 0x00000001); //?
	GPUCMD_AddWrite(GPUREG_DEPTHMAP_SCALE, f32tof24(zScale));
	GPUCMD_AddWrite(GPUREG_DEPTHMAP_OFFSET, f32tof24(zOffset));
}

void GPU_SetAlphaTest(bool enable, GPU_TESTFUNC function, u8 ref)
{
	GPUCMD_AddWrite(GPUREG_ALPHATEST_CONFIG, (enable&1)|((function&7)<<4)|(ref<<8));
}

void GPU_SetStencilTest(bool enable, GPU_TESTFUNC function, u8 ref, u8 input_mask, u8 write_mask)
{
	GPUCMD_AddWrite(GPUREG_STENCILTEST_CONFIG, (enable&1)|((function&7)<<4)|(write_mask<<8)|(ref<<16)|(input_mask<<24));
}

void GPU_SetStencilOp(GPU_STENCILOP sfail, GPU_STENCILOP dfail, GPU_STENCILOP pass)
{
	GPUCMD_AddWrite(GPUREG_STENCILOP_CONFIG, sfail | (dfail << 4) | (pass << 8));
}

void GPU_SetDepthTestAndWriteMask(bool enable, GPU_TESTFUNC function, GPU_WRITEMASK writemask)
{
	GPUCMD_AddWrite(GPUREG_DEPTHTEST_CONFIG, (enable&1)|((function&7)<<4)|(writemask<<8));
}

void GPU_SetAlphaBlending(GPU_BLENDEQUATION colorEquation, GPU_BLENDEQUATION alphaEquation, 
	GPU_BLENDFACTOR colorSrc, GPU_BLENDFACTOR colorDst, 
	GPU_BLENDFACTOR alphaSrc, GPU_BLENDFACTOR alphaDst)
{
	GPUCMD_AddWrite(GPUREG_BLEND_CONFIG, colorEquation | (alphaEquation<<8) | (colorSrc<<16) | (colorDst<<20) | (alphaSrc<<24) | (alphaDst<<28));
	GPUCMD_AddMaskedWrite(GPUREG_COLOROUTPUT_CONFIG, 0x2, 0x00000100);
}

void GPU_SetColorLogicOp(GPU_LOGICOP op)
{
	GPUCMD_AddWrite(GPUREG_COLORLOGICOP_CONFIG, op);
	GPUCMD_AddMaskedWrite(GPUREG_COLOROUTPUT_CONFIG, 0x2, 0x00000000);
}

void GPU_SetBlendingColor(u8 r, u8 g, u8 b, u8 a)
{
	GPUCMD_AddWrite(GPUREG_BLEND_COLOR, r | (g << 8) | (b << 16) | (a << 24));
}

void GPU_SetTextureEnable(GPU_TEXUNIT units)
{
	GPUCMD_AddMaskedWrite(GPUREG_006F, 0x2, units<<8); 			// enables texcoord outputs
	GPUCMD_AddWrite(GPUREG_TEXUNITS_CONFIG, 0x00011000|units);	// enables texture units
}

void GPU_SetTexture(GPU_TEXUNIT unit, u32* data, u16 width, u16 height, u32 param, GPU_TEXCOLOR colorType)
{
	switch (unit)
	{
	case GPU_TEXUNIT0:
		GPUCMD_AddWrite(GPUREG_TEXUNIT0_TYPE, colorType);
		GPUCMD_AddWrite(GPUREG_TEXUNIT0_LOC, ((u32)data)>>3);
		GPUCMD_AddWrite(GPUREG_TEXUNIT0_DIM, (width<<16)|height);
		GPUCMD_AddWrite(GPUREG_TEXUNIT0_PARAM, param);
		break;
		
	case GPU_TEXUNIT1:
		GPUCMD_AddWrite(GPUREG_TEXUNIT1_TYPE, colorType);
		GPUCMD_AddWrite(GPUREG_TEXUNIT1_LOC, ((u32)data)>>3);
		GPUCMD_AddWrite(GPUREG_TEXUNIT1_DIM, (width<<16)|height);
		GPUCMD_AddWrite(GPUREG_TEXUNIT1_PARAM, param);
		break;
		
	case GPU_TEXUNIT2:
		GPUCMD_AddWrite(GPUREG_TEXUNIT2_TYPE, colorType);
		GPUCMD_AddWrite(GPUREG_TEXUNIT2_LOC, ((u32)data)>>3);
		GPUCMD_AddWrite(GPUREG_TEXUNIT2_DIM, (width<<16)|height);
		GPUCMD_AddWrite(GPUREG_TEXUNIT2_PARAM, param);
		break;
	}
}

void GPU_SetTextureBorderColor(GPU_TEXUNIT unit,u32 borderColor)
{
	switch (unit)
	{
	case GPU_TEXUNIT0:
		GPUCMD_AddWrite(GPUREG_TEXUNIT0_BORDER_COLOR, borderColor);
		break;
		
	case GPU_TEXUNIT1:
		GPUCMD_AddWrite(GPUREG_TEXUNIT1_BORDER_COLOR, borderColor);
		break;
		
	case GPU_TEXUNIT2:
		GPUCMD_AddWrite(GPUREG_TEXUNIT2_BORDER_COLOR, borderColor);
		break;
	}
}

const u8 GPU_FORMATSIZE[4]={1,1,2,4};

void GPU_SetAttributeBuffers(u8 totalAttributes, u32* baseAddress, u64 attributeFormats, u16 attributeMask, u64 attributePermutation, u8 numBuffers, u32 bufferOffsets[], u64 bufferPermutations[], u8 bufferNumAttributes[])
{
	u32 param[0x28];

	memset(param, 0x00, 0x28*4);

	param[0x0]=((u32)baseAddress)>>3;
	param[0x1]=attributeFormats&0xFFFFFFFF;
	param[0x2]=((totalAttributes-1)<<28)|((attributeMask&0xFFF)<<16)|((attributeFormats>>32)&0xFFFF);

	int i, j;
	u8 sizeTable[0xC];
	for(i=0;i<totalAttributes;i++)
	{
		u8 v=attributeFormats&0xF;
		sizeTable[i]=GPU_FORMATSIZE[v&3]*((v>>2)+1);
		attributeFormats>>=4;
	}

	for(i=0;i<numBuffers;i++)
	{
		u16 stride=0;
		param[3*(i+1)+0]=bufferOffsets[i];
		param[3*(i+1)+1]=bufferPermutations[i]&0xFFFFFFFF;
		for(j=0;j<bufferNumAttributes[i];j++)stride+=sizeTable[(bufferPermutations[i]>>(4*j))&0xF];
		param[3*(i+1)+2]=(bufferNumAttributes[i]<<28)|((stride&0xFFF)<<16)|((bufferPermutations[i]>>32)&0xFFFF);
	}

	GPUCMD_AddIncrementalWrites(GPUREG_ATTRIBBUFFERS_LOC, param, 0x00000027);

	GPUCMD_AddMaskedWrite(GPUREG_VSH_INPUTBUFFER_CONFIG, 0xB, 0xA0000000|(totalAttributes-1));
	GPUCMD_AddWrite(GPUREG_0242, (totalAttributes-1));

	GPUCMD_AddIncrementalWrites(GPUREG_VSH_ATTRIBUTES_PERMUTATION_LOW, ((u32[]){attributePermutation&0xFFFFFFFF, (attributePermutation>>32)&0xFFFF}), 2);
}

void GPU_SetAttributeBuffersAddress(u32* baseAddress)
{
	GPUCMD_AddWrite(GPUREG_ATTRIBBUFFERS_LOC, ((u32)baseAddress)>>3);
}

void GPU_SetFaceCulling(GPU_CULLMODE mode)
{
	GPUCMD_AddWrite(GPUREG_FACECULLING_CONFIG, mode&0x3); 
}

void GPU_SetCombinerBufferWrite(u8 rgb_config, u8 alpha_config)
{
    GPUCMD_AddMaskedWrite(GPUREG_TEXENV_BUFFER_CONFIG, 0x2, (rgb_config << 8) | (alpha_config << 12));
}

const u8 GPU_TEVID[]={0xC0,0xC8,0xD0,0xD8,0xF0,0xF8};

void GPU_SetTexEnv(u8 id, u16 rgbSources, u16 alphaSources, u16 rgbOperands, u16 alphaOperands, GPU_COMBINEFUNC rgbCombine, GPU_COMBINEFUNC alphaCombine, u32 constantColor)
{
	if(id>6)return;
	u32 param[0x5];
	memset(param, 0x00, 5*4);

	param[0x0]=(alphaSources<<16)|(rgbSources);
	param[0x1]=(alphaOperands<<12)|(rgbOperands);
	param[0x2]=(alphaCombine<<16)|(rgbCombine);
	param[0x3]=constantColor;
	param[0x4]=0x00000000; // ?

	GPUCMD_AddIncrementalWrites(GPUREG_0000|GPU_TEVID[id], param, 0x00000005);
}

void GPU_DrawArray(GPU_Primitive_t primitive, u32 first, u32 count)
{
	//set primitive type
	GPUCMD_AddMaskedWrite(GPUREG_PRIMITIVE_CONFIG, 0x2, primitive);
	GPUCMD_AddMaskedWrite(GPUREG_025F, 0x2, 0x00000001);
	//index buffer address register should be cleared (except bit 31) before drawing
	GPUCMD_AddWrite(GPUREG_INDEXBUFFER_CONFIG, 0x80000000);
	//pass number of vertices
	GPUCMD_AddWrite(GPUREG_NUMVERTICES, count);
	//set first vertex
	GPUCMD_AddWrite(GPUREG_DRAW_VERTEX_OFFSET, first);

	//all the following except 0x000F022E might be useless
	GPUCMD_AddMaskedWrite(GPUREG_0253, 0x1, 0x00000001);
	GPUCMD_AddMaskedWrite(GPUREG_0245, 0x1, 0x00000000);
	GPUCMD_AddWrite(GPUREG_DRAWARRAYS, 0x00000001);
	GPUCMD_AddMaskedWrite(GPUREG_0245, 0x1, 0x00000001);
	GPUCMD_AddWrite(GPUREG_0231, 0x00000001);
	GPUCMD_AddWrite(GPUREG_0111, 0x00000001);
}

void GPU_DrawElements(GPU_Primitive_t primitive, u32* indexArray, u32 n)
{
	//set primitive type
	GPUCMD_AddMaskedWrite(GPUREG_PRIMITIVE_CONFIG, 0x2, primitive);
	GPUCMD_AddMaskedWrite(GPUREG_025F, 0x2, 0x00000001);
	//index buffer (TODO : support multiple types)
	GPUCMD_AddWrite(GPUREG_INDEXBUFFER_CONFIG, 0x80000000|((u32)indexArray));
	//pass number of vertices
	GPUCMD_AddWrite(GPUREG_NUMVERTICES, n);
    
	GPUCMD_AddWrite(GPUREG_DRAW_VERTEX_OFFSET, 0x00000000);
    
	GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG, 0x2, 0x00000100);
	GPUCMD_AddMaskedWrite(GPUREG_0253, 0x2, 0x00000100);

	GPUCMD_AddMaskedWrite(GPUREG_0245, 0x1, 0x00000000);
	GPUCMD_AddWrite(GPUREG_DRAWELEMENTS, 0x00000001);
	GPUCMD_AddMaskedWrite(GPUREG_0245, 0x1, 0x00000001);
	GPUCMD_AddWrite(GPUREG_0231, 0x00000001);
	
	// CHECKME: does this one also require command 0x0111 at the end?
}

void GPU_FinishDrawing()
{
	GPUCMD_AddWrite(GPUREG_0111, 0x00000001);
	GPUCMD_AddWrite(GPUREG_0110, 0x00000001); 
	GPUCMD_AddWrite(GPUREG_0063, 0x00000001);
}

void GPU_SetShaderOutmap(u32 outmapData[8])
{
	GPUCMD_AddMaskedWrite(GPUREG_PRIMITIVE_CONFIG, 0x1, outmapData[0]-1);
	GPUCMD_AddIncrementalWrites(GPUREG_SH_OUTMAP_TOTAL, outmapData, 8);
}

void GPU_SendShaderCode(GPU_SHADER_TYPE type, u32* data, u16 offset, u16 length)
{
	if(!data)return;

	int regOffset=(type==GPU_GEOMETRY_SHADER)?(-0x30):(0x0);

	GPUCMD_AddWrite(GPUREG_VSH_CODETRANSFER_CONFIG+regOffset, offset);

	int i;
	for(i=0;i<length;i+=0x80)GPUCMD_AddWrites(GPUREG_VSH_CODETRANSFER_DATA+regOffset, &data[i], ((length-i)<0x80)?(length-i):0x80);

	GPUCMD_AddWrite(GPUREG_VSH_CODETRANSFER_END+regOffset, 0x00000001);
}

void GPU_SendOperandDescriptors(GPU_SHADER_TYPE type, u32* data, u16 offset, u16 length)
{
	if(!data)return;

	int regOffset=(type==GPU_GEOMETRY_SHADER)?(-0x30):(0x0);

	GPUCMD_AddWrite(GPUREG_VSH_OPDESCS_CONFIG+regOffset, offset);

	GPUCMD_AddWrites(GPUREG_VSH_OPDESCS_DATA+regOffset, data, length);
}
