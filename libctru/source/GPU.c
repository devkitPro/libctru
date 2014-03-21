#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/GSP.h>
#include <ctr/GX.h>
#include <ctr/GPU.h>
#include <ctr/svc.h>

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

void GPUCMD_Run(u32* gxbuf)
{
	GX_SetCommandList_First(gxbuf, gpuCmdBuf, gpuCmdBufOffset*4, NULL, 0, NULL, 0);
	GX_SetCommandList_Last(gxbuf, gpuCmdBuf, gpuCmdBufOffset*4, 0x0);
}

void GPUCMD_Add(u32 cmd, u32* param, u32 paramlength)
{
	u32 zero=0x0;

	if(!param || !paramlength)
	{
		paramlength=1;
		param=&zero;
	}

	if(!gpuCmdBuf || gpuCmdBufOffset+paramlength+1>gpuCmdBufSize)return;

	paramlength--;
	cmd|=(paramlength&0x7ff)<<20;

	gpuCmdBuf[gpuCmdBufOffset]=param[0];
	gpuCmdBuf[gpuCmdBufOffset+1]=cmd;

	if(paramlength)memcpy(&gpuCmdBuf[gpuCmdBufOffset+2], &param[1], paramlength*4);

	gpuCmdBufOffset+=paramlength+2;

	if(paramlength&1)gpuCmdBuf[gpuCmdBufOffset++]=0x00000000; //alignment
}

void GPUCMD_AddSingleParam(u32 cmd, u32 param)
{
	GPUCMD_Add(cmd, &param, 1);
}

void GPUCMD_Finalize()
{
	GPUCMD_AddSingleParam(0x000F0111, 0x00000001);
	GPUCMD_AddSingleParam(0x000F0110, 0x00000001);
	GPUCMD_AddSingleParam(0x000F0010, 0x12345678);
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

	GPUCMD_AddSingleParam(0x000D0080, 0x00011000);

	for(i=0x1;i<0xC;i++)GPUCMD_AddSingleParam(0x000F0080+i, 0x00000000);
	GPUCMD_AddSingleParam(0x000F008C, 0x00FF0000);
	GPUCMD_AddSingleParam(0x000F008D, 0x00000000);
	GPUCMD_AddSingleParam(0x000F008E, 0x00000000);

	for(i=0x0;i<0xF;i++)GPUCMD_AddSingleParam(0x000F0090+i, 0x00000000);

	GPUCMD_AddSingleParam(0x00010245, 0x00000001);
	GPUCMD_AddSingleParam(0x00010244, 0x00000000);
	GPUCMD_AddSingleParam(0x00080289, 0x80000000);
	GPUCMD_AddSingleParam(0x000B0229, 0x00000000);

	GPUCMD_AddSingleParam(0x000F0252, 0x00000000);
	GPUCMD_AddSingleParam(0x000F0251, 0x00000000);
	GPUCMD_AddSingleParam(0x000F0254, 0x00000000);
	GPUCMD_AddSingleParam(0x00010253, 0x00000000);

	GPUCMD_AddSingleParam(0x000F0242, 0x00000000);
	GPUCMD_AddSingleParam(0x000F024A, 0x00000000);

	GPUCMD_AddSingleParam(0x0005025E, 0x00000000);

	GPUCMD_Add(0x800F0101, zero, 0x00000007);

	GPUCMD_AddSingleParam(0x000F011F, 0x00010140);
	GPUCMD_AddSingleParam(0x000F0100, 0x00E40100);
	GPUCMD_AddSingleParam(0x000F0101, 0x01010000);
	GPUCMD_AddSingleParam(0x000F0107, 0x00001F40);
	GPUCMD_AddSingleParam(0x000F0105, 0xFF00FF10);

	GPUCMD_AddSingleParam(0x00010061, 0x00000003);
	GPUCMD_AddSingleParam(0x00010062, 0x00000000);

	GPUCMD_AddSingleParam(0x000F0065, 0x00000000);
	GPUCMD_AddSingleParam(0x000F0066, 0x00000000);
	GPUCMD_AddSingleParam(0x000F0067, 0x00000000);

	GPUCMD_AddSingleParam(0x00010118, 0x00000000);
	GPUCMD_AddSingleParam(0x000F011B, 0x00000000);

	GPUCMD_AddSingleParam(0x0007006A, 0x00FFFFFF);

	GPUCMD_AddSingleParam(0x000F0102, 0x00000003);

	GPUCMD_AddSingleParam(0x00080126, 0x03000000);

	GPUCMD_Add(0x800F0040, zero, 0x00000010);

	param[0x0]=0x1F1F1F1F;
	param[0x1]=0x1F1F1F1F;
	param[0x2]=0x1F1F1F1F;
	param[0x3]=0x1F1F1F1F;
	param[0x4]=0x1F1F1F1F;
	param[0x5]=0x1F1F1F1F;
	param[0x6]=0x1F1F1F1F;
	GPUCMD_Add(0x800F0050, param, 0x00000007);

	GPUCMD_AddSingleParam(0x000F0058, 0x00000100);
	GPUCMD_AddSingleParam(0x000F004C, 0x00000001);
	GPUCMD_AddSingleParam(0x000F006F, 0x00000000);

	GPUCMD_AddSingleParam(0x00020060, 0x00000000);
	GPUCMD_AddSingleParam(0x000C0069, 0x00020000);

	GPUCMD_AddSingleParam(0x000F0113, 0x0000000F);
	GPUCMD_AddSingleParam(0x000F0112, 0x0000000F);
	GPUCMD_AddSingleParam(0x000F0114, 0x00000003);
	GPUCMD_AddSingleParam(0x000F0115, 0x00000003);

	GPUCMD_AddSingleParam(0x000F01C5, 0x00000000);

	for(i=0;i<32;i++)GPUCMD_Add(0x800F01C8, zero, 0x00000008);
	GPUCMD_AddSingleParam(0x000F01C5, 0x00000100);
	for(i=0;i<32;i++)GPUCMD_Add(0x800F01C8, zero, 0x00000008);
	GPUCMD_AddSingleParam(0x000F01C5, 0x00000200);
	for(i=0;i<32;i++)GPUCMD_Add(0x800F01C8, zero, 0x00000008);
	GPUCMD_AddSingleParam(0x000F01C5, 0x00000300);
	for(i=0;i<32;i++)GPUCMD_Add(0x800F01C8, zero, 0x00000008);
	GPUCMD_AddSingleParam(0x000F01C5, 0x00000400);
	for(i=0;i<32;i++)GPUCMD_Add(0x800F01C8, zero, 0x00000008);
	GPUCMD_AddSingleParam(0x000F01C5, 0x00000500);
	for(i=0;i<32;i++)GPUCMD_Add(0x800F01C8, zero, 0x00000008);
	GPUCMD_AddSingleParam(0x000F01C5, 0x00000600);
	for(i=0;i<32;i++)GPUCMD_Add(0x800F01C8, zero, 0x00000008);
	GPUCMD_AddSingleParam(0x000F0290, 0x80000000);
	for(i=0;i<48;i++)GPUCMD_Add(0x800F0291, zero, 0x00000008);
	GPUCMD_AddSingleParam(0x000F02CB, 0x00000000);
	for(i=0;i<4;i++)GPUCMD_Add(0x000F02CC, zero, 0x00000080);
	GPUCMD_AddSingleParam(0x000F029B, 0x00000200);

	for(i=0;i<28;i++)GPUCMD_Add(0x000F029C, zero, 0x00000080);

	GPUCMD_AddSingleParam(0x000F02BF, 0x00000000);
	GPUCMD_AddSingleParam(0x000F02B1, 0x00000000);
	GPUCMD_AddSingleParam(0x000F02B2, 0x00000000);
	GPUCMD_AddSingleParam(0x000F02B3, 0x00000000);
	GPUCMD_AddSingleParam(0x000F02B4, 0x00000000);

	param[0x0]=0xFFFFFFFF;
	param[0x1]=0xFFFFFFFF;
	GPUCMD_Add(0x800F028B, param, 0x00000002);

	GPUCMD_Add(0x800F0205, zero, 0x00000024);

	for(i=0;i<gpuResetSequenceLength;i++)GPUCMD_AddSingleParam(gpuResetSequence[i*2],gpuResetSequence[i*2+1]);

	GPUCMD_Finalize();
	GPUCMD_Run(gpuBuf);
}

void GPU_SetUniform(u32 startreg, u32* data, u32 numreg)
{
	if(!data)return;

	GPUCMD_AddSingleParam(0x000F02C0, 0x80000000|startreg);
	GPUCMD_Add(0x000F02C1, data, numreg*4);
}

//TODO : fix
u32 f32tof24(float f)
{
	if(!f)return 0;
	u32 v=*((u32*)&f);
	u8 s=v>>31;
	u32 exp=((v>>23)&0xFF)-0x40;
	u32 man=(v>>7)&0xFFFF;

	if(exp>=0)return man|(exp<<16)|(s<<23);
	else return s<<23;
}

u32 computeInvValue(u32 val)
{
	//usual values
	if(val==240)return 0x38111111;
	if(val==480)return 0x37111111;
	if(val==400)return 0x3747ae14;
	//but let's not limit ourselves to the usual
	float fval=2.0/val;
	u32 tmp1,tmp2;
	u32 tmp3=*((u32*)&fval);
	tmp1=(tmp3<<9)>>9;
	tmp2=tmp3&(~0x80000000);
	if(tmp2)
	{
		tmp1=(tmp3<<9)>>9;
		int tmp=((tmp3<<1)>>24)-0x40;
		if(tmp<0)return ((tmp3>>31)<<30)<<1;
		else tmp2=tmp;
	}
	tmp3>>=31;
	return (tmp1|(tmp2<<23)|(tmp3<<30))<<1;
}

//takes PAs as arguments
void GPU_SetViewport(u32* depthBuffer, u32* colorBuffer, u32 x, u32 y, u32 w, u32 h)
{
	u32 param[0x4];
	float fw=(float)w;
	float fh=(float)h;

	GPUCMD_AddSingleParam(0x000F0111, 0x00000001);
	GPUCMD_AddSingleParam(0x000F0110, 0x00000001);

	u32 f116e=0x01000000|(((h-1)&0xFFF)<<12)|(w&0xFFF);

	param[0x0]=((u32)depthBuffer)>>3;
	param[0x1]=((u32)colorBuffer)>>3;
	param[0x2]=f116e;
	GPUCMD_Add(0x800F011C, param, 0x00000003);

	GPUCMD_AddSingleParam(0x000F006E, f116e);
	GPUCMD_AddSingleParam(0x000F0116, 0x00000003); //depth buffer format
	GPUCMD_AddSingleParam(0x000F0117, 0x00000002); //color buffer format
	GPUCMD_AddSingleParam(0x000F011B, 0x00000000); //?

	param[0x0]=f32tof24(fw/2);
	param[0x1]=computeInvValue(fw);
	param[0x2]=f32tof24(fh/2);
	param[0x3]=computeInvValue(fh);
	GPUCMD_Add(0x800F0041, param, 0x00000004);

	GPUCMD_AddSingleParam(0x000F0068, (y<<16)|(x&0xFFFF));

	param[0x0]=0x00000000;
	param[0x1]=0x00000000;
	param[0x2]=((h-1)<<16)|((w-1)&0xFFFF);
	GPUCMD_Add(0x800F0065, param, 0x00000003);

	//enable depth buffer
	param[0x0]=0x00000000;
	param[0x1]=0x0000000F;
	param[0x2]=0x00000002;
	param[0x3]=0x00000002;
	GPUCMD_Add(0x800F0112, param, 0x00000004);
}

void GPU_DepthRange(float nearVal, float farVal)
{
	GPUCMD_AddSingleParam(0x000F006D, 0x00000001); //?
	GPUCMD_AddSingleParam(0x000F004D, f32tof24(nearVal));
	GPUCMD_AddSingleParam(0x000F004E, f32tof24(farVal));
}

void GPU_SetStencilTest(bool enable, GPU_TESTFUNC function, u8 ref)
{
	GPUCMD_AddSingleParam(0x000F0105, (enable&1)|((function&7)<<4)|(ref<<8));
}

void GPU_SetDepthTest(bool enable, GPU_TESTFUNC function, u8 ref)
{
	GPUCMD_AddSingleParam(0x000F0107, (enable&1)|((function&7)<<4)|(ref<<8));
}

void GPU_SetTexture(u32* data, u16 width, u16 height, u32 param, GPU_TEXCOLOR colorType)
{
	GPUCMD_AddSingleParam(0x000F008E, colorType);
	GPUCMD_AddSingleParam(0x000F0085, ((u32)data)>>3);
	GPUCMD_AddSingleParam(0x000F0082, (width)|(height<<16));
	GPUCMD_AddSingleParam(0x000F0083, param);
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

	GPUCMD_Add(0x800F0200, param, 0x00000027);

	GPUCMD_AddSingleParam(0x000B02B9, 0xA0000000|(totalAttributes-1));
	GPUCMD_AddSingleParam(0x000F0242, (totalAttributes-1));

	GPUCMD_AddSingleParam(0x000F02BB, attributePermutation&0xFFFFFFFF);
	GPUCMD_AddSingleParam(0x000F02BC, (attributePermutation>>32)&0xFFFF);
}

void GPU_SetFaceCulling(GPU_CULLMODE mode)
{
	GPUCMD_AddSingleParam(0x000F0040, mode&0x3); 
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

	GPUCMD_Add(0x800F0000|GPU_TEVID[id], param, 0x00000005);
}
