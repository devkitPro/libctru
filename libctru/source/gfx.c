#include <stdlib.h>
#include <stdio.h>
#include <3ds/types.h>
#include <3ds/gfx.h>
#include <3ds/svc.h>
#include <3ds/synchronization.h>
#include <3ds/allocator/linear.h>
#include <3ds/allocator/mappable.h>
#include <3ds/allocator/vram.h>
#include <3ds/gpu/gx.h>

GSPGPU_FramebufferInfo topFramebufferInfo, bottomFramebufferInfo;

u8 gfxThreadID;
u8* gfxSharedMemory;

u8* gfxTopLeftFramebuffers[2];
u8* gfxTopRightFramebuffers[2];
u8* gfxBottomFramebuffers[2];
u32 gfxTopFramebufferMaxSize;
u32 gfxBottomFramebufferMaxSize;

static bool enable3d;
static u8 currentBuffer[2];
static int doubleBuf[2] = {1,1};

Handle gspEvent, gspSharedMemHandle;

static GSPGPU_FramebufferFormats topFormat = GSP_BGR8_OES;
static GSPGPU_FramebufferFormats botFormat = GSP_BGR8_OES;

static GSPGPU_FramebufferInfo* const framebufferInfoSt[] = { &topFramebufferInfo, &bottomFramebufferInfo };

static void (*screenFree)(void *) = NULL;
static void *(*screenAlloc)(size_t) = NULL;

void gfxSet3D(bool enable)
{
	enable3d=enable;
}

bool gfxIs3D(void)
{
	return enable3d;
}

u32 __get_bytes_per_pixel(GSPGPU_FramebufferFormats format) {
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

void gfxSetScreenFormat(gfxScreen_t screen, GSPGPU_FramebufferFormats format) {
	if (screenAlloc == NULL || screenFree == NULL) return;
	if(screen==GFX_TOP)
	{
		u32 topSize = 400 * 240 * __get_bytes_per_pixel(format);
		if (gfxTopFramebufferMaxSize < topSize)
		{
			screenFree(gfxTopLeftFramebuffers[0]);
			screenFree(gfxTopLeftFramebuffers[1]);
			screenFree(gfxTopRightFramebuffers[0]);
			screenFree(gfxTopRightFramebuffers[1]);
			gfxTopLeftFramebuffers[0]=screenAlloc(topSize);
			gfxTopLeftFramebuffers[1]=screenAlloc(topSize);
			gfxTopRightFramebuffers[0]=screenAlloc(topSize);
			gfxTopRightFramebuffers[1]=screenAlloc(topSize);
			gfxTopFramebufferMaxSize = topSize;
		}
		topFormat = format;
	}else{
		u32 bottomSize = 320 * 240 * __get_bytes_per_pixel(format);
		if (gfxBottomFramebufferMaxSize < bottomSize)
		{
			screenFree(gfxBottomFramebuffers[0]);
			screenFree(gfxBottomFramebuffers[1]);
			gfxBottomFramebuffers[0]=screenAlloc(bottomSize);
			gfxBottomFramebuffers[1]=screenAlloc(bottomSize);
			gfxBottomFramebufferMaxSize = bottomSize;
		}
		botFormat = format;
	}
}

GSPGPU_FramebufferFormats gfxGetScreenFormat(gfxScreen_t screen) {
	if(screen==GFX_TOP)
		return topFormat;
	else
		return botFormat;
}

void gfxSetDoubleBuffering(gfxScreen_t screen, bool doubleBuffering) {
	doubleBuf[screen] = doubleBuffering ? 1 : 0; // make sure they're the integer values '1' and '0'
}

static void gfxSetFramebufferInfo(gfxScreen_t screen, u8 id)
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

static void gfxWriteFramebufferInfo(gfxScreen_t screen)
{
	s32* framebufferInfoHeader=(s32*)(gfxSharedMemory+0x200+gfxThreadID*0x80);
	if(screen==GFX_BOTTOM)framebufferInfoHeader+=0x10;
	GSPGPU_FramebufferInfo* framebufferInfo=(GSPGPU_FramebufferInfo*)&framebufferInfoHeader[1];
	union
	{
		s32 header;
		struct { u8 swap, update; };
	} info;
	info.header = __ldrex(framebufferInfoHeader);
	info.swap = !info.swap;
	u8 pos = info.swap;
	info.update = 1;
	framebufferInfo[pos]=*framebufferInfoSt[screen];
	while (__strex(framebufferInfoHeader,info.header))
	{
		info.header = __ldrex(framebufferInfoHeader);
		info.swap = pos;
		info.update = 1;
	}
}

void gfxInit(GSPGPU_FramebufferFormats topFormat, GSPGPU_FramebufferFormats bottomFormat, bool vrambuffers)
{
	if (vrambuffers)
	{
		screenAlloc=vramAlloc;
		screenFree=vramFree;

	} else {

		screenAlloc=linearAlloc;
		screenFree=linearFree;
	}

	gspInit();

	gfxSharedMemory=(u8*)mappableAlloc(0x1000);

	GSPGPU_AcquireRight(0x0);

	//setup our gsp shared mem section
	svcCreateEvent(&gspEvent, RESET_ONESHOT);
	GSPGPU_RegisterInterruptRelayQueue(gspEvent, 0x1, &gspSharedMemHandle, &gfxThreadID);
	svcMapMemoryBlock(gspSharedMemHandle, (u32)gfxSharedMemory, 0x3, 0x10000000);

	// default gspHeap configuration :
	//		topleft1  0x00000000-0x00046500
	//		topleft2  0x00046500-0x0008CA00
	//		bottom1   0x0008CA00-0x000C4E00
	//		bottom2   0x000C4E00-0x000FD200
	//	 if 3d enabled :
	//		topright1 0x000FD200-0x00143700
	//		topright2 0x00143700-0x00189C00
	u32 topSize = 400 * 240 * __get_bytes_per_pixel(topFormat);
	u32 bottomSize = 320 * 240 * __get_bytes_per_pixel(bottomFormat);

	gfxTopLeftFramebuffers[0]=screenAlloc(topSize);
	gfxTopLeftFramebuffers[1]=screenAlloc(topSize);
	gfxBottomFramebuffers[0]=screenAlloc(bottomSize);
	gfxBottomFramebuffers[1]=screenAlloc(bottomSize);
	gfxTopRightFramebuffers[0]=screenAlloc(topSize);
	gfxTopRightFramebuffers[1]=screenAlloc(topSize);
	gfxTopFramebufferMaxSize = topSize;
	gfxBottomFramebufferMaxSize = bottomSize;

	enable3d=false;

	//set requested modes
	gfxSetScreenFormat(GFX_TOP,topFormat);
	gfxSetScreenFormat(GFX_BOTTOM,bottomFormat);

	//initialize framebuffer info structures
	gfxSetFramebufferInfo(GFX_TOP, 0);
	gfxSetFramebufferInfo(GFX_BOTTOM, 0);

	//GSP shared mem : 0x2779F000
	gxCmdBuf=(u32*)(gfxSharedMemory+0x800+gfxThreadID*0x200);

	currentBuffer[0]=0;
	currentBuffer[1]=0;

	// Initialize event handler and wait for VBlank
	gspInitEventHandler(gspEvent, (vu8*) gfxSharedMemory, gfxThreadID);
	gspWaitForVBlank();

	GSPGPU_SetLcdForceBlack(0x0);
}

void gfxInitDefault(void) {
	gfxInit(GSP_BGR8_OES,GSP_BGR8_OES,false);
}

void gfxExit(void)
{
	if (screenFree == NULL) return;

	// Exit event handler
	gspExitEventHandler();

	// Free framebuffers
	screenFree(gfxTopRightFramebuffers[1]);
	screenFree(gfxTopRightFramebuffers[0]);
	screenFree(gfxBottomFramebuffers[1]);
	screenFree(gfxBottomFramebuffers[0]);
	screenFree(gfxTopLeftFramebuffers[1]);
	screenFree(gfxTopLeftFramebuffers[0]);

	//unmap GSP shared mem
	svcUnmapMemoryBlock(gspSharedMemHandle, (u32)gfxSharedMemory);

	GSPGPU_UnregisterInterruptRelayQueue();

	svcCloseHandle(gspSharedMemHandle);
	if(gfxSharedMemory != NULL)
	{
		mappableFree(gfxSharedMemory);
		gfxSharedMemory = NULL;
	}

	svcCloseHandle(gspEvent);

	GSPGPU_ReleaseRight();

	gspExit();

	screenFree = NULL;
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

void gfxFlushBuffers(void)
{
	u32 topSize = 400 * 240 * __get_bytes_per_pixel(gfxGetScreenFormat(GFX_TOP));
	u32 bottomSize = 320 * 240 * __get_bytes_per_pixel(gfxGetScreenFormat(GFX_BOTTOM));

	GSPGPU_FlushDataCache(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), topSize);
	if(enable3d)GSPGPU_FlushDataCache(gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL), topSize);
	GSPGPU_FlushDataCache(gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL), bottomSize);
}

void gfxConfigScreen(gfxScreen_t scr, bool immediate)
{
	currentBuffer[scr]^=doubleBuf[scr];
	gfxSetFramebufferInfo(scr, currentBuffer[scr]);
	if (immediate)
		GSPGPU_SetBufferSwap(scr, framebufferInfoSt[scr]);
	else
		gfxWriteFramebufferInfo(scr);
}

void gfxSwapBuffers(void)
{
	gfxConfigScreen(GFX_TOP, true);
	gfxConfigScreen(GFX_BOTTOM, true);
}

void gfxSwapBuffersGpu(void)
{
	gfxConfigScreen(GFX_TOP, false);
	gfxConfigScreen(GFX_BOTTOM, false);
}
