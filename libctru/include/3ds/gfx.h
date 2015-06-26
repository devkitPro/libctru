/**
 * @file gfx.h
 * @brief LCD Screens manipulation
 *
 * This header provides functions to configure and manipulate the two screens, including double buffering and 3D activation.
 * It is mainly an abstraction over the gsp service.
 */

#pragma once
#include <3ds/types.h>
#include <3ds/services/gsp.h>

#define RGB565(r,g,b)  (((b)&0x1f)|(((g)&0x3f)<<5)|(((r)&0x1f)<<11))
#define RGB8_to_565(r,g,b)  (((b)>>3)&0x1f)|((((g)>>2)&0x3f)<<5)|((((r)>>3)&0x1f)<<11)

typedef enum
{
	GFX_TOP = 0,
	GFX_BOTTOM = 1
}gfxScreen_t;

/**
 * @brief Side of top screen framebuffer.
 *
 * This is to be used only when the 3D is enabled.
 * Use only GFX_LEFT if this concerns the bottom screen or if 3D is disabled.
 */
typedef enum
{
	GFX_LEFT = 0, ///< Left eye framebuffer
	GFX_RIGHT = 1,///< Right eye framebuffer
	// GFX_BOTTOM = 0
}gfx3dSide_t;


///@name System related
///@{

/**
 * @brief Initializes the LCD framebuffers with default parameters
 *
 * By default ctrulib will configure the LCD framebuffers with the @ref GSP_BGR8_OES format in linear memory.
 * This is the same as calling : @code gfxInit(GSP_BGR8_OES,GSP_BGR8_OES,false); @endcode
 *
 * @note You should always call @ref gfxExit once done to free the memory and services
 */
void gfxInitDefault();

/**
 * @brief Initializes the LCD framebuffers
 * @brief topFormat The format of the top screen framebuffers
 * @brief bottomFormat The format of the bottom screen framebuffers
 *
 * This function will allocate the memory for the framebuffers and open a gsp service session.
 * It will also bind the newly allocated framebuffers to the LCD screen and setup the VBlank event.
 *
 * The 3D stereoscopic display is will be disabled.
 *
 * @note Even if the double buffering is disabled, it will allocate two buffer per screen.
 * @note You should always call @ref gfxExit once done to free the memory and services
 */
void gfxInit(GSP_FramebufferFormats topFormat, GSP_FramebufferFormats bottomFormat, bool vrambuffers);

/**
 * @brief Closes the gsp service and frees the framebuffers.
 *
 * Just call it when you're done.
 */
void gfxExit();
///@}

///@name Control
///@{
/**
 * @brief Enables the 3D stereoscopic effect.
 * @param enable Enables the 3D effect if true, disables it if false.
 */
void gfxSet3D(bool enable);

/**
 * @brief Changes the color format of a screen
 * @param screen The screen of which format should be changed
 * @param format One of the gsp pixel formats.
 */
void gfxSetScreenFormat(gfxScreen_t screen, GSP_FramebufferFormats format);

/**
 * @brief Gets a screen pixel format.
 * @return the pixel format of the chosen screen set by ctrulib.
 */
GSP_FramebufferFormats gfxGetScreenFormat(gfxScreen_t screen);

/**
 * @brief Enables the ctrulib double buffering
 *
 * ctrulib is by default using a double buffering scheme.
 * If you do not want to swap one of the screen framebuffers when @ref gfxSwapBuffers or @ref gfxSwapBuffers is called,
 * then you have to disable double buffering.
 *
 * It is however recommended to call @ref gfxSwapBuffers even if double buffering is disabled
 * for both screens if you want to keep the gsp configuration up to date.
 */
void gfxSetDoubleBuffering(gfxScreen_t screen, bool doubleBuffering);

/**
 * @brief Flushes the current framebuffers
 *
 * Use this if the data within your framebuffers changes a lot and that you want to make sure everything was updated correctly.
 * This shouldn't be needed and has a significant overhead.
 */
void gfxFlushBuffers();

/**
 * @brief Swaps the buffers and sets the gsp state
 *
 * This is to be called to update the gsp state and swap the framebuffers.
 * LCD rendering should start as soon as the gsp state is set.
 * When using the GPU, call @ref gfxSwapBuffers instead.
 */
void gfxSwapBuffers();

/**
 * @brief Swaps the framebuffers
 *
 * This is the version to be used with the GPU since the GPU will use the gsp shared memory,
 * so the gsp state mustn't be set directly by the user.
 */
void gfxSwapBuffersGpu();

///@}


///@name Helper
///@{
/**
 * @brief Retrieves a framebuffer information
 * @param width Pointer that will hold the width of the framebuffer in pixels
 * @param height Pointer that will hold the height of the framebuffer in pixels
 * @return a pointer to the current framebuffer of the choosen screen
 *
 * Please remember that the returned pointer will change after each call to gfxSwapBuffers if double buffering is enabled.
 */
u8* gfxGetFramebuffer(gfxScreen_t screen, gfx3dSide_t side, u16* width, u16* height);
///@}

//global variables
extern u8* gfxTopLeftFramebuffers[2];
extern u8* gfxTopRightFramebuffers[2];
extern u8* gfxBottomFramebuffers[2];
extern u32* gxCmdBuf;
