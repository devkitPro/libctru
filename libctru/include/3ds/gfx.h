#pragma once
#include <3ds/types.h>
#include <3ds/services/gsp.h>

#define RGB565(r,g,b)  (((b)&0x1f)|(((g)&0x3f)<<5)|(((r)&0x1f)<<11))

typedef enum
{
	GFX_TOP = 0,
	GFX_BOTTOM = 1
}gfxScreen_t;

typedef enum
{
	GFX_LEFT = 0,
	GFX_RIGHT = 1,
	// GFX_BOTTOM = 0
}gfx3dSide_t;

//system stuff
void gfxInit();
void gfxExit();

//control stuff
void gfxSet3D(bool enable);
void gfxSetScreenFormat(gfxScreen_t screen, GSP_FramebufferFormats format);
void gfxSetDoubleBuffering(gfxScreen_t screen, bool doubleBuffering);
void gfxFlushBuffers();
void gfxSwapBuffers();
void gfxSwapBuffersGpu();

//helper stuff
u8* gfxGetFramebuffer(gfxScreen_t screen, gfx3dSide_t side, u16* width, u16* height);

//global variables
extern u8* gfxTopLeftFramebuffers[2];
extern u8* gfxTopRightFramebuffers[2];
extern u8* gfxBottomFramebuffers[2];
extern u32* gxCmdBuf;
