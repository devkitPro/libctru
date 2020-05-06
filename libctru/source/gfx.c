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

	u8 pos = 1 - *(u8*)framebufferInfoHeader;
	framebufferInfo[pos]=*framebufferInfoSt[screen];
	__dsb();

	union
	{
		s32 header;
		struct { u8 swap, update; };
	} info;

	do
	{
		info.header = __ldrex(framebufferInfoHeader);
		info.swap = pos;
		info.update = 1;
	} while (__strex(framebufferInfoHeader, info.header));
}

static inline void gfxWriteGxReg(u32 offset, u32 data)
{
	GSPGPU_WriteHWRegs(0x400000 + offset, &data, 4);
}
static inline void gfxWriteGxRegMasked(u32 offset, u32 data, u32 mask)
{
	GSPGPU_WriteHWRegsWithMask(0x400000 + offset, &data, 4, &mask, 4);
}

static void gfxGxHwInit(void)
{
	// SDK apps have this exact sequence (except for GPUREG_START_DRAW_FUNC0)

	// Some GPU-internal init registers
	gfxWriteGxReg(0x1000, 0);
	gfxWriteGxReg(0x1080, 0x12345678);
	gfxWriteGxReg(0x10C0, 0xFFFFFFF0);
	gfxWriteGxReg(0x10D0, 1);
	// Ensure GPUREG_START_DRAW_FUNC0 starts off in configuration mode
	gfxWriteGxReg(0x1914, 1);

	// Top screen LCD configuration, see https://www.3dbrew.org/wiki/GPU/External_Registers#LCD_Source_Framebuffer_Setup

	// Top screen sync registers:
	gfxWriteGxReg(0x0400, 0x1C2);
	gfxWriteGxReg(0x0404, 0xD1);
	gfxWriteGxReg(0x0408, 0x1C1);
	gfxWriteGxReg(0x040C, 0x1C1);
	gfxWriteGxReg(0x0410, 0);
	gfxWriteGxReg(0x0414, 0xCF);
	gfxWriteGxReg(0x0418, 0xD1);
	gfxWriteGxReg(0x041C, (0x1C5 << 16) | 0x1C1);
	gfxWriteGxReg(0x0420, 0x10000);
	gfxWriteGxReg(0x0424, 0x19D);
	gfxWriteGxReg(0x0428, 2);
	gfxWriteGxReg(0x042C, 0x192);
	gfxWriteGxReg(0x0430, 0x192);
	gfxWriteGxReg(0x0434, 0x192);
	gfxWriteGxReg(0x0438, 1);
	gfxWriteGxReg(0x043C, 2);
	gfxWriteGxReg(0x0440, (0x196 << 16) | 0x192);
	gfxWriteGxReg(0x0444, 0);
	gfxWriteGxReg(0x0448, 0);

	// Top screen fb geometry
	gfxWriteGxReg(0x045C, (400 << 16) | 240); // dimensions
	gfxWriteGxReg(0x0460, (0x1C1 << 16) | 0xD1);
	gfxWriteGxReg(0x0464, (0x192 << 16) | 2);

	// Top screen framebuffer format (initial)
	gfxWriteGxReg(0x0470, 0x80340);

	// Top screen unknown reg @ 0x9C
	gfxWriteGxReg(0x049C, 0);

	// Bottom screen LCD configuration

	// Bottom screen sync registers:
	gfxWriteGxReg(0x0500, 0x1C2);
	gfxWriteGxReg(0x0504, 0xD1);
	gfxWriteGxReg(0x0508, 0x1C1);
	gfxWriteGxReg(0x050C, 0x1C1);
	gfxWriteGxReg(0x0510, 0xCD);
	gfxWriteGxReg(0x0514, 0xCF);
	gfxWriteGxReg(0x0518, 0xD1);
	gfxWriteGxReg(0x051C, (0x1C5 << 16) | 0x1C1);
	gfxWriteGxReg(0x0520, 0x10000);
	gfxWriteGxReg(0x0524, 0x19D);
	gfxWriteGxReg(0x0528, 0x52);
	gfxWriteGxReg(0x052C, 0x192);
	gfxWriteGxReg(0x0530, 0x192);
	gfxWriteGxReg(0x0534, 0x4F);
	gfxWriteGxReg(0x0538, 0x50);
	gfxWriteGxReg(0x053C, 0x52);
	gfxWriteGxReg(0x0540, (0x198 << 16) | 0x194);
	gfxWriteGxReg(0x0544, 0);
	gfxWriteGxReg(0x0548, 0x11);

	// Bottom screen fb geometry
	gfxWriteGxReg(0x055C, (320 << 16) | 240); // dimensions
	gfxWriteGxReg(0x0560, (0x1C1 << 16) | 0xD1);
	gfxWriteGxReg(0x0564, (0x192 << 16) | 0x52);

	// Bottom screen framebuffer format (initial)
	gfxWriteGxReg(0x0570, 0x80300);

	// Bottom screen unknown reg @ 0x9C
	gfxWriteGxReg(0x059C, 0);

	// Initial, blank framebuffer (top left A/B, bottom A/B, top right A/B)
	gfxWriteGxReg(0x0468, 0x18300000);
	gfxWriteGxReg(0x046C, 0x18300000);
	gfxWriteGxReg(0x0568, 0x18300000);
	gfxWriteGxReg(0x056C, 0x18300000);
	gfxWriteGxReg(0x0494, 0x18300000);
	gfxWriteGxReg(0x0498, 0x18300000);

	// Framebuffer select: A
	gfxWriteGxReg(0x0478, 1);
	gfxWriteGxReg(0x0578, 1);

	// Clear DMA transfer (PPF) "transfer finished" bit
	gfxWriteGxRegMasked(0x0C18, 0, 0xFF00);

	// GX_GPU_CLK |= 0x70000 (value is 0x100 when gsp starts, enough to at least display framebuffers & have memory fill work)
	// This enables the clock to some GPU components
	gfxWriteGxReg(0x0004, 0x70100);

	// Clear Memory Fill (PSC0 and PSC1) "busy" and "finished" bits
	gfxWriteGxRegMasked(0x001C, 0, 0xFF);
	gfxWriteGxRegMasked(0x002C, 0, 0xFF);

	// More init registers
	gfxWriteGxReg(0x0050, 0x22221200);
	gfxWriteGxRegMasked(0x0054, 0xFF2, 0xFFFF);

	// Enable some LCD clocks (?) (unsure)
	gfxWriteGxReg(0x0474, 0x10501);
	gfxWriteGxReg(0x0574, 0x10501);
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

	GSPGPU_AcquireRight(0);

	//setup our gsp shared mem section
	svcCreateEvent(&gspEvent, RESET_ONESHOT);

	// The 0x2A07 success code is returned only for the very first call to that function (globally)
	if (GSPGPU_RegisterInterruptRelayQueue(gspEvent, 0x1, &gspSharedMemHandle, &gfxThreadID) == 0x2A07)
		gfxGxHwInit();

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
