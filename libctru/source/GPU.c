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

u32 f32tof24(float f)
{
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
	GPUCMD_AddSingleParam(0x000F0116, 0x00000003); //?
	GPUCMD_AddSingleParam(0x000F0117, 0x00000002); //?
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
}
