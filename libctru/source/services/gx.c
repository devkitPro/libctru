/*
  gx.c _ Sending GPU requests via GSP shared memory.
*/

#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/gpu/gx.h>
#include <3ds/services/gsp.h>

u32* gxCmdBuf;

Result GX_RequestDma(u32* gxbuf, u32* src, u32* dst, u32 length)
{
	if(!gxbuf)gxbuf=gxCmdBuf;

	u32 gxCommand[0x8];
	gxCommand[0]=0x00; //CommandID
	gxCommand[1]=(u32)src; //source address
	gxCommand[2]=(u32)dst; //destination address
	gxCommand[3]=length; //size
	gxCommand[4]=gxCommand[5]=gxCommand[6]=gxCommand[7]=0x0;

	return GSPGPU_SubmitGxCommand(gxbuf, gxCommand, NULL);
}

Result GX_SetCommandList_Last(u32* gxbuf, u32* buf0a, u32 buf0s, u8 flags)
{
	if(!gxbuf)gxbuf=gxCmdBuf;

	u32 gxCommand[0x8];
	gxCommand[0]=0x01; //CommandID
	gxCommand[1]=(u32)buf0a; //buf0 address
	gxCommand[2]=(u32)buf0s; //buf0 size
	gxCommand[3]=flags&1; //written to GSP module state
	gxCommand[4]=gxCommand[5]=gxCommand[6]=0x0;
	gxCommand[7]=(flags>>1)&1; //when non-zero, call svcFlushProcessDataCache() with the specified buffer

	return GSPGPU_SubmitGxCommand(gxbuf, gxCommand, NULL);
}

Result GX_SetMemoryFill(u32* gxbuf, u32* buf0a, u32 buf0v, u32* buf0e, u16 control0, u32* buf1a, u32 buf1v, u32* buf1e, u16 control1)
{
	if(!gxbuf)gxbuf=gxCmdBuf;

	u32 gxCommand[0x8];
	// gxCommand[0]=0x02; //CommandID
	gxCommand[0]=0x01000102; //CommandID
	gxCommand[1]=(u32)buf0a; //buf0 address
	gxCommand[2]=buf0v; //buf0 value
	gxCommand[3]=(u32)buf0e; //buf0 end addr
	gxCommand[4]=(u32)buf1a; //buf1 address
	gxCommand[5]=buf1v; //buf1 value
	gxCommand[6]=(u32)buf1e; //buf1 end addr
	gxCommand[7]=(control0)|(control1<<16);

	return GSPGPU_SubmitGxCommand(gxbuf, gxCommand, NULL);
}

// Flags, for applications this is 0x1001000 for the main screen, and 0x1000 for the sub screen.
Result GX_SetDisplayTransfer(u32* gxbuf, u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 flags)
{
	if(!gxbuf)gxbuf=gxCmdBuf;

	u32 gxCommand[0x8];
	gxCommand[0]=0x03; //CommandID
	gxCommand[1]=(u32)inadr;
	gxCommand[2]=(u32)outadr;
	gxCommand[3]=indim;
	gxCommand[4]=outdim;
	gxCommand[5]=flags;
	gxCommand[6]=gxCommand[7]=0x0;

	return GSPGPU_SubmitGxCommand(gxbuf, gxCommand, NULL);
}

Result GX_SetTextureCopy(u32* gxbuf, u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 size, u32 flags)
{
	if(!gxbuf)gxbuf=gxCmdBuf;

	u32 gxCommand[0x8];
	gxCommand[0]=0x04; //CommandID
	gxCommand[1]=(u32)inadr;
	gxCommand[2]=(u32)outadr;
	gxCommand[3]=size;
	gxCommand[4]=indim;
	gxCommand[5]=outdim;
	gxCommand[6]=flags;
	gxCommand[7]=0x0;

	return GSPGPU_SubmitGxCommand(gxbuf, gxCommand, NULL);
}

Result GX_SetCommandList_First(u32* gxbuf, u32* buf0a, u32 buf0s, u32* buf1a, u32 buf1s, u32* buf2a, u32 buf2s)
{
	if(!gxbuf)gxbuf=gxCmdBuf;

	u32 gxCommand[0x8];
	gxCommand[0]=0x05; //CommandID
	gxCommand[1]=(u32)buf0a; //buf0 address
	gxCommand[2]=(u32)buf0s; //buf0 size
	gxCommand[3]=(u32)buf1a; //buf1 address
	gxCommand[4]=(u32)buf1s; //buf1 size
	gxCommand[5]=(u32)buf2a; //buf2 address
	gxCommand[6]=(u32)buf2s; //buf2 size
	gxCommand[7]=0x0;

	return GSPGPU_SubmitGxCommand(gxbuf, gxCommand, NULL);
}
