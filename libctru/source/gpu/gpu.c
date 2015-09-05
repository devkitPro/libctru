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
