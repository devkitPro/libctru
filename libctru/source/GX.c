#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/GSP.h>
#include <ctr/GX.h>
#include <ctr/svc.h>

Result GX_RequestDma(u32* gxbuf, u32* src, u32* dst, u32 length)
{
	u32 gxCommand[0x8];
	gxCommand[0]=0x00; //CommandID
	gxCommand[1]=(u32)src; //source address
	gxCommand[2]=(u32)dst; //destination address
	gxCommand[3]=length; //size
	gxCommand[4]=gxCommand[5]=gxCommand[6]=gxCommand[7]=0x0;

	return GSPGPU_submitGxCommand(gxbuf, gxCommand, NULL);
}

Result GX_SetCommandList_Last(u32* gxbuf, u32* buf0a, u32 buf0s, u8 flags)
{
	u32 gxCommand[0x8];
	gxCommand[0]=0x01; //CommandID
	gxCommand[1]=(u32)buf0a; //buf0 address
	gxCommand[2]=(u32)buf0s; //buf0 size
	gxCommand[3]=flags&1; //written to GSP module state
	gxCommand[4]=gxCommand[5]=gxCommand[6]=0x0;
	gxCommand[7]=(flags>>1)&1; //when non-zero, call svcFlushProcessDataCache() with the specified buffer

	return GSPGPU_submitGxCommand(gxbuf, gxCommand, NULL);
}

Result GX_SetMemoryFill(u32* gxbuf, u32* buf0a, u32 buf0s, u32 buf0d, u16 width0, u32* buf1a, u32 buf1s, u32 buf1d, u16 width1)
{
	u32 gxCommand[0x8];
	gxCommand[0]=0x02; //CommandID
	gxCommand[1]=(u32)buf0a; //buf0 address
	gxCommand[2]=buf0s; //buf0 size
	gxCommand[3]=buf0d; //buf0 data
	gxCommand[4]=(u32)buf1a; //buf1 address
	gxCommand[5]=buf1s; //buf1 size
	gxCommand[6]=buf1d; //buf1 data
	gxCommand[7]=(width0)|(width1<<16);

	return GSPGPU_submitGxCommand(gxbuf, gxCommand, NULL);
}

// Flags, for applications this is 0x1001000 for the main screen, and 0x1000 for the sub screen.
Result GX_SetDisplayTransfer(u32* gxbuf, u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 flags)
{
	u32 gxCommand[0x8];
	gxCommand[0]=0x03; //CommandID
	gxCommand[1]=(u32)inadr;
	gxCommand[2]=(u32)outadr;
	gxCommand[3]=indim;
	gxCommand[4]=outdim;
	gxCommand[5]=flags;
	gxCommand[6]=gxCommand[7]=0x0;

	return GSPGPU_submitGxCommand(gxbuf, gxCommand, NULL);
}

Result GX_SetCommandList_First(u32* gxbuf, u32* buf0a, u32 buf0s, u32* buf1a, u32 buf1s, u32* buf2a, u32 buf2s)
{
	u32 gxCommand[0x8];
	gxCommand[0]=0x05; //CommandID
	gxCommand[1]=(u32)buf0a; //buf0 address
	gxCommand[2]=(u32)buf0s; //buf0 size
	gxCommand[3]=(u32)buf1a; //buf1 address
	gxCommand[4]=(u32)buf1s; //buf1 size
	gxCommand[5]=(u32)buf2a; //buf2 address
	gxCommand[6]=(u32)buf2s; //buf2 size
	gxCommand[7]=0x0;

	return GSPGPU_submitGxCommand(gxbuf, gxCommand, NULL);
}
