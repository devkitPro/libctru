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
	u32 cmd[2];

	GPUCMD_AddSingleParam(0x000F0111, 0x00000001);
	GPUCMD_AddSingleParam(0x000F0110, 0x00000001);
	GPUCMD_AddSingleParam(0x000F0010, 0x12345678);
}
