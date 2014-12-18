#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/gfx.h>
#include <3ds/svc.h>
#include <3ds/linear.h>

GSP_FramebufferInfo topFramebufferInfo, bottomFramebufferInfo;

u8 gfxThreadID;
u8* gfxSharedMemory;

u8* gfxTopLeftFramebuffers[2];
u8* gfxTopRightFramebuffers[2];
u8* gfxBottomFramebuffers[2];

static bool enable3d;
static u8 currentBuffer[2];
static int doubleBuf[2] = {1,1};

Handle gspEvent, gspSharedMemHandle;

static GSP_FramebufferFormats topFormat = GSP_BGR8_OES;
static GSP_FramebufferFormats botFormat = GSP_BGR8_OES;

void gfxSet3D(bool enable)
{
	enable3d=enable;
}

void gfxSetScreenFormat(gfxScreen_t screen, GSP_FramebufferFormats format) {
    if(screen==GFX_TOP)
        topFormat = format;
    else
        botFormat = format;
}

void gfxSetDoubleBuffering( gfxScreen_t screen, bool doubleBuffering) {
	doubleBuf[screen] = doubleBuffering ? 1 : 0; // make sure they're the integer values '1' and '0'
}

static u32 __get_bytes_per_pixel(GSP_FramebufferFormats format) {
    switch(format) {
    case GSP_RGBA8_OES:
        return 4;
    case GSP_BGR8_OES:
        return 3;
    case GSP_RGB565_OES:
    case GSP_RGB5_A1_OES:
    case GSP_RGBA4_OES:
        return 2;
    }

    return 3;
}

void gfxSetFramebufferInfo(gfxScreen_t screen, u8 id)
{
	if(screen==GFX_TOP)
	{
		topFramebufferInfo.active_framebuf=id;
		topFramebufferInfo.framebuf0_vaddr=(u32*)gfxTopLeftFramebuffers[id];
		if(enable3d)topFramebufferInfo.framebuf1_vaddr=(u32*)gfxTopRightFramebuffers[id];
		else topFramebufferInfo.framebuf1_vaddr=topFramebufferInfo.framebuf0_vaddr;
		topFramebufferInfo.framebuf_widthbytesize=240*__get_bytes_per_pixel(topFormat);
		u8 bit5=(enable3d!=0);
		topFramebufferInfo.format=((1)<<8)|((1^bit5)<<6)|((bit5)<<5)|topFormat;
		topFramebufferInfo.framebuf_dispselect=id;
		topFramebufferInfo.unk=0x00000000;
	}else{
		bottomFramebufferInfo.active_framebuf=id;
		bottomFramebufferInfo.framebuf0_vaddr=(u32*)gfxBottomFramebuffers[id];
		bottomFramebufferInfo.framebuf1_vaddr=0x00000000;
		bottomFramebufferInfo.framebuf_widthbytesize=240*__get_bytes_per_pixel(botFormat);
		bottomFramebufferInfo.format=botFormat;
		bottomFramebufferInfo.framebuf_dispselect=id;
		bottomFramebufferInfo.unk=0x00000000;
	}
}

void gfxWriteFramebufferInfo(gfxScreen_t screen)
{
	u8* framebufferInfoHeader=gfxSharedMemory+0x200+gfxThreadID*0x80;
	if(screen==GFX_BOTTOM)framebufferInfoHeader+=0x40;
	GSP_FramebufferInfo* framebufferInfo=(GSP_FramebufferInfo*)&framebufferInfoHeader[0x4];
	framebufferInfoHeader[0x0]^=doubleBuf[screen];
	framebufferInfo[framebufferInfoHeader[0x0]]=(screen==GFX_TOP)?(topFramebufferInfo):(bottomFramebufferInfo);
	framebufferInfoHeader[0x1]=1;
}

void gfxInit()
{
	gspInit();

	gfxSharedMemory=(u8*)0x10002000;

	GSPGPU_AcquireRight(NULL, 0x0);

	//setup our gsp shared mem section
	svcCreateEvent(&gspEvent, 0x0);
	GSPGPU_RegisterInterruptRelayQueue(NULL, gspEvent, 0x1, &gspSharedMemHandle, &gfxThreadID);
	svcMapMemoryBlock(gspSharedMemHandle, (u32)gfxSharedMemory, 0x3, 0x10000000);

	// default gspHeap configuration :
	//		topleft1  0x00000000-0x00046500
	//		topleft2  0x00046500-0x0008CA00
	//		bottom1   0x0008CA00-0x000C4E00
	//		bottom2   0x000C4E00-0x000FD200
	//	 if 3d enabled :
	//		topright1 0x000FD200-0x00143700
	//		topright2 0x00143700-0x00189C00

	gfxTopLeftFramebuffers[0]=linearAlloc(0x46500);
	gfxTopLeftFramebuffers[1]=linearAlloc(0x46500);
	gfxBottomFramebuffers[0]=linearAlloc(0x38400);
	gfxBottomFramebuffers[1]=linearAlloc(0x38400);
	gfxTopRightFramebuffers[0]=linearAlloc(0x46500);
	gfxTopRightFramebuffers[1]=linearAlloc(0x46500);
	enable3d=false;

	//initialize framebuffer info structures
	gfxSetFramebufferInfo(GFX_TOP, 0);
	gfxSetFramebufferInfo(GFX_BOTTOM, 0);

	//GSP shared mem : 0x2779F000
	gxCmdBuf=(u32*)(gfxSharedMemory+0x800+gfxThreadID*0x200);

	currentBuffer[0]=0;
	currentBuffer[1]=0;

	// Initialize event handler and wait for VBlank
	gspInitEventHandler(gspEvent, (vu8*)gfxSharedMemory, gfxThreadID);
	gspWaitForVBlank();

	GSPGPU_SetLcdForceBlack(NULL, 0x0);
}

void gfxExit()
{
	// Exit event handler
	gspExitEventHandler();

	// Free framebuffers
	linearFree(gfxTopRightFramebuffers[1]);
	linearFree(gfxTopRightFramebuffers[0]);
	linearFree(gfxBottomFramebuffers[1]);
	linearFree(gfxBottomFramebuffers[0]);
	linearFree(gfxTopLeftFramebuffers[1]);
	linearFree(gfxTopLeftFramebuffers[0]);

	//unmap GSP shared mem
	svcUnmapMemoryBlock(gspSharedMemHandle, 0x10002000);

	GSPGPU_UnregisterInterruptRelayQueue(NULL);

	svcCloseHandle(gspSharedMemHandle);
	svcCloseHandle(gspEvent);

	GSPGPU_ReleaseRight(NULL);

	gspExit();
}

u8* gfxGetFramebuffer(gfxScreen_t screen, gfx3dSide_t side, u16* width, u16* height)
{
	if(width)*width=240;

	if(screen==GFX_TOP)
	{
		if(height)*height=400;
		return (side==GFX_LEFT || !enable3d)?(gfxTopLeftFramebuffers[currentBuffer[0]^doubleBuf[0]]):(gfxTopRightFramebuffers[currentBuffer[0]^doubleBuf[0]]);
	}else{
		if(height)*height=320;
		return gfxBottomFramebuffers[currentBuffer[1]^doubleBuf[1]];
	}
}

void gfxFlushBuffers()
{
	GSPGPU_FlushDataCache(NULL, gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 0x46500);
	if(enable3d)GSPGPU_FlushDataCache(NULL, gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL), 0x46500);
	GSPGPU_FlushDataCache(NULL, gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL), 0x38400);
}

void gfxSwapBuffers()
{
	currentBuffer[0]^=doubleBuf[0];
	currentBuffer[1]^=doubleBuf[1];
	gfxSetFramebufferInfo(GFX_TOP, currentBuffer[0]);
	gfxSetFramebufferInfo(GFX_BOTTOM, currentBuffer[1]);
	GSPGPU_SetBufferSwap(NULL, GFX_TOP, &topFramebufferInfo);
	GSPGPU_SetBufferSwap(NULL, GFX_BOTTOM, &bottomFramebufferInfo);
}

void gfxSwapBuffersGpu()
{
	currentBuffer[0]^=doubleBuf[0];
	currentBuffer[1]^=doubleBuf[1];
	gfxSetFramebufferInfo(GFX_TOP, currentBuffer[0]);
	gfxSetFramebufferInfo(GFX_BOTTOM, currentBuffer[1]);
	gfxWriteFramebufferInfo(GFX_TOP);
	gfxWriteFramebufferInfo(GFX_BOTTOM);
}
