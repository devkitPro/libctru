#include <3ds/types.h>
#include <3ds/allocator/linear.h>
#include <3ds/allocator/vram.h>
#include <3ds/services/gspgpu.h>
#include <3ds/gfx.h>

static u8* gfxTopFramebuffers[2];
static u8* gfxBottomFramebuffers[2];
static u32 gfxTopFramebufferMaxSize;
static u32 gfxBottomFramebufferMaxSize;
static GSPGPU_FramebufferFormat gfxFramebufferFormats[2];

static enum {
	MODE_2D   = 0,
	MODE_3D   = 1,
	MODE_WIDE = 2,
} gfxTopMode;
static bool gfxIsVram;
static u8 gfxCurBuf[2];
static u8 gfxIsDoubleBuf[2];

static void (*screenFree)(void *);
static void *(*screenAlloc)(size_t);

void gfxSet3D(bool enable)
{
	gfxTopMode = enable ? MODE_3D : MODE_2D;
}

bool gfxIs3D(void)
{
	return gfxTopMode == MODE_3D;
}

void gfxSetWide(bool enable)
{
	gfxTopMode = enable ? MODE_WIDE : MODE_2D;
}

bool gfxIsWide(void)
{
	return gfxTopMode == MODE_WIDE;
}

void gfxSetScreenFormat(gfxScreen_t screen, GSPGPU_FramebufferFormat format)
{
	u32 reqSize = GSP_SCREEN_WIDTH * gspGetBytesPerPixel(format);
	u8** framebuffers;
	u32* maxSize;

	if (screen == GFX_TOP)
	{
		reqSize *= GSP_SCREEN_HEIGHT_TOP_2X;
		framebuffers = gfxTopFramebuffers;
		maxSize = &gfxTopFramebufferMaxSize;
	}
	else // GFX_BOTTOM
	{
		reqSize *= GSP_SCREEN_HEIGHT_BOTTOM;
		framebuffers = gfxBottomFramebuffers;
		maxSize = &gfxBottomFramebufferMaxSize;
	}

	if (*maxSize < reqSize)
	{
		if (framebuffers[0]) screenFree(framebuffers[0]);
		if (framebuffers[1]) screenFree(framebuffers[1]);
		framebuffers[0] = (u8*)screenAlloc(reqSize);
		framebuffers[1] = (u8*)screenAlloc(reqSize);
		*maxSize = reqSize;
	}

	gfxFramebufferFormats[screen] = format;
}

GSPGPU_FramebufferFormat gfxGetScreenFormat(gfxScreen_t screen)
{
	return gfxFramebufferFormats[screen];
}

void gfxSetDoubleBuffering(gfxScreen_t screen, bool enable)
{
	gfxIsDoubleBuf[screen] = enable ? 1 : 0; // make sure they're the integer values '1' and '0'
}

static void gfxPresentFramebuffer(gfxScreen_t screen, u8 id, bool hasStereo)
{
	u32 stride = GSP_SCREEN_WIDTH*gspGetBytesPerPixel(gfxFramebufferFormats[screen]);
	u32 mode = gfxFramebufferFormats[screen];

	const u8 *fb_a, *fb_b;
	if (screen == GFX_TOP)
	{
		fb_a = gfxTopFramebuffers[id];
		switch (gfxTopMode)
		{
			default:
			case MODE_2D:
				mode |= BIT(6);
				fb_b = fb_a;
				break;
			case MODE_3D:
				mode |= BIT(5);
				fb_b = hasStereo ? (fb_a + gfxTopFramebufferMaxSize/2) : fb_a;
				break;
			case MODE_WIDE:
				fb_b = fb_a;
				break;
		}
	}
	else
	{
		fb_a = gfxBottomFramebuffers[id];
		fb_b = fb_a;
	}

	if (!gfxIsVram)
		mode |= 1<<8;
	else
		mode |= 3<<8;

	gspPresentBuffer(screen, id, fb_a, fb_b, stride, mode);
}

void gfxInit(GSPGPU_FramebufferFormat topFormat, GSPGPU_FramebufferFormat bottomFormat, bool vrambuffers)
{
	if (vrambuffers)
	{
		screenAlloc = vramAlloc;
		screenFree = vramFree;
		gfxIsVram = true;
	}
	else
	{
		screenAlloc = linearAlloc;
		screenFree = linearFree;
		gfxIsVram = false;
	}

	// Initialize GSP
	gspInit();

	// Initialize configuration
	gfxSet3D(false);
	gfxSetScreenFormat(GFX_TOP, topFormat);
	gfxSetScreenFormat(GFX_BOTTOM, bottomFormat);
	gfxSetDoubleBuffering(GFX_TOP, true);
	gfxSetDoubleBuffering(GFX_BOTTOM, true);

	// Present the framebuffers
	gfxCurBuf[0] = gfxCurBuf[1] = 0;
	gfxPresentFramebuffer(GFX_TOP, 0, false);
	gfxPresentFramebuffer(GFX_BOTTOM, 0, false);

	// Wait for VBlank and turn the LCD on
	gspWaitForVBlank();
	GSPGPU_SetLcdForceBlack(0x0);
}

void gfxInitDefault(void)
{
	gfxInit(GSP_BGR8_OES,GSP_BGR8_OES,false);
}

void gfxExit(void)
{
	if (screenFree == NULL)
		return;

	if (gspHasGpuRight())
	{
		// Wait for VBlank and turn the LCD off
		gspWaitForVBlank();
		GSPGPU_SetLcdForceBlack(0x1);
	}

	// Free framebuffers
	screenFree(gfxTopFramebuffers[0]);
	screenFree(gfxTopFramebuffers[1]);
	screenFree(gfxBottomFramebuffers[0]);
	screenFree(gfxBottomFramebuffers[1]);
	gfxTopFramebuffers[0] = gfxTopFramebuffers[1] = NULL;
	gfxBottomFramebuffers[0] = gfxBottomFramebuffers[1] = NULL;
	gfxTopFramebufferMaxSize = gfxBottomFramebufferMaxSize = 0;

	// Deinitialize GSP
	gspExit();

	screenFree = NULL;
}

u8* gfxGetFramebuffer(gfxScreen_t screen, gfx3dSide_t side, u16* width, u16* height)
{
	unsigned id = gfxCurBuf[screen]^gfxIsDoubleBuf[screen];
	unsigned scr_width = GSP_SCREEN_WIDTH;
	unsigned scr_height;
	u8* fb;

	if (screen == GFX_TOP)
	{
		fb = gfxTopFramebuffers[id];
		scr_height = GSP_SCREEN_HEIGHT_TOP;
		switch (gfxTopMode)
		{
			default:
			case MODE_2D:
				break;
			case MODE_3D:
				if (side != GFX_LEFT)
					fb += gfxTopFramebufferMaxSize/2;
				break;
			case MODE_WIDE:
				scr_height = GSP_SCREEN_HEIGHT_TOP_2X;
				break;
		}
	}
	else // GFX_BOTTOM
	{
		fb = gfxBottomFramebuffers[id];
		scr_height = GSP_SCREEN_HEIGHT_BOTTOM;
	}

	if (width)
		*width = scr_width;
	if (height)
		*height = scr_height;

	return fb;
}

void gfxFlushBuffers(void)
{
	const u32 baseSize   = GSP_SCREEN_WIDTH * gspGetBytesPerPixel(gfxGetScreenFormat(GFX_TOP));
	const u32 topSize    = GSP_SCREEN_HEIGHT_TOP * baseSize;
	const u32 topSize2x  = GSP_SCREEN_HEIGHT_TOP_2X * baseSize;
	const u32 bottomSize = GSP_SCREEN_HEIGHT_BOTTOM * baseSize;

	GSPGPU_FlushDataCache(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), gfxTopMode == MODE_WIDE ? topSize2x : topSize);
	if (gfxTopMode == MODE_3D)
		GSPGPU_FlushDataCache(gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL), topSize);
	GSPGPU_FlushDataCache(gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL), bottomSize);
}

void gfxScreenSwapBuffers(gfxScreen_t scr, bool hasStereo)
{
	gfxCurBuf[scr] ^= gfxIsDoubleBuf[scr];
	gfxPresentFramebuffer(scr, gfxCurBuf[scr], hasStereo);
}

void gfxConfigScreen(gfxScreen_t scr, bool immediate)
{
	gfxScreenSwapBuffers(scr, true);
}

void gfxSwapBuffers(void)
{
	gfxScreenSwapBuffers(GFX_TOP, true);
	gfxScreenSwapBuffers(GFX_BOTTOM, true);
}

void gfxSwapBuffersGpu(void)
{
	gfxScreenSwapBuffers(GFX_TOP, true);
	gfxScreenSwapBuffers(GFX_BOTTOM, true);
}
