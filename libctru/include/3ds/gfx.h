/**
 * @file gfx.h
 * @brief Simple framebuffer API
 *
 * This API provides basic functionality needed to bring up framebuffers for both screens,
 * as well as managing display mode (stereoscopic 3D) and double buffering.
 * It is mainly an abstraction over the gsp service.
 *
 * Please note that the 3DS uses *portrait* screens rotated 90 degrees counterclockwise.
 * Width/height refer to the physical dimensions of the screen; that is, the top screen
 * is 240 pixels wide and 400 pixels tall; while the bottom screen is 240x320.
 */
#pragma once

#include <3ds/types.h>
#include <3ds/services/gspgpu.h>

/// Converts red, green, and blue components to packed RGB565.
#define RGB565(r,g,b)  (((b)&0x1f)|(((g)&0x3f)<<5)|(((r)&0x1f)<<11))

/// Converts packed RGB8 to packed RGB565.
#define RGB8_to_565(r,g,b)  (((b)>>3)&0x1f)|((((g)>>2)&0x3f)<<5)|((((r)>>3)&0x1f)<<11)

/// Screen IDs.
typedef enum {
	GFX_TOP    = GSP_SCREEN_TOP,    ///< Top screen
	GFX_BOTTOM = GSP_SCREEN_BOTTOM, ///< Bottom screen
} gfxScreen_t;

/**
 * @brief Top screen framebuffer side.
 *
 * This is only meaningful when stereoscopic 3D is enabled on the top screen.
 * In any other case, use \ref GFX_LEFT.
 */
typedef enum {
	GFX_LEFT  = 0, ///< Left eye framebuffer
	GFX_RIGHT = 1, ///< Right eye framebuffer
} gfx3dSide_t;

///@name Initialization and deinitialization
///@{

/**
 * @brief Initializes the LCD framebuffers with default parameters
 * This is equivalent to calling: @code gfxInit(GSP_BGR8_OES,GSP_BGR8_OES,false); @endcode
 */
void gfxInitDefault(void);

/**
 * @brief Initializes the LCD framebuffers.
 * @param topFormat The format of the top screen framebuffers.
 * @param bottomFormat The format of the bottom screen framebuffers.
 * @param vramBuffers Whether to allocate the framebuffers in VRAM.
 *
 * This function allocates memory for the framebuffers in the specified memory region.
 * Initially, stereoscopic 3D is disabled and double buffering is enabled.
 *
 * @note This function internally calls \ref gspInit.
 */
void gfxInit(GSPGPU_FramebufferFormat topFormat, GSPGPU_FramebufferFormat bottomFormat, bool vrambuffers);

/**
 * @brief Deinitializes and frees the LCD framebuffers.
 * @note This function internally calls \ref gspExit.
 */
void gfxExit(void);

///@}

///@name Control
///@{

/**
 * @brief Enables or disables the 3D stereoscopic effect on the top screen.
 * @param enable Pass true to enable, false to disable.
 * @note Stereoscopic 3D is disabled by default.
 */
void gfxSet3D(bool enable);

/**
 * @brief Retrieves the status of the 3D stereoscopic effect on the top screen.
 * @return true if 3D enabled, false otherwise.
 */
bool gfxIs3D(void);

/**
 * @brief Retrieves the status of the 800px (double-height) high resolution display mode of the top screen.
 * @return true if wide mode enabled, false otherwise.
 */
bool gfxIsWide(void);

/**
 * @brief Enables or disables the 800px (double-height) high resolution display mode of the top screen.
 * @param enable Pass true to enable, false to disable.
 * @note Wide mode is disabled by default.
 * @note Wide and stereoscopic 3D modes are mutually exclusive.
 * @note In wide mode pixels are not square, since scanlines are half as tall as they normally are.
 * @warning Wide mode does not work on Old 2DS consoles (however it does work on New 2DS XL consoles).
 */
void gfxSetWide(bool enable);

/**
 * @brief Changes the pixel format of a screen.
 * @param screen Screen ID (see \ref gfxScreen_t)
 * @param format Pixel format (see \ref GSPGPU_FramebufferFormat)
 * @note If the currently allocated framebuffers are too small for the specified format,
 *       they are freed and new ones are reallocated.
 */
void gfxSetScreenFormat(gfxScreen_t screen, GSPGPU_FramebufferFormat format);

/**
 * @brief Retrieves the current pixel format of a screen.
 * @param screen Screen ID (see \ref gfxScreen_t)
 * @return Pixel format (see \ref GSPGPU_FramebufferFormat)
 */
GSPGPU_FramebufferFormat gfxGetScreenFormat(gfxScreen_t screen);

/**
 * @brief Enables or disables double buffering on a screen.
 * @param screen Screen ID (see \ref gfxScreen_t)
 * @param enable Pass true to enable, false to disable.
 * @note Double buffering is enabled by default.
 */
void gfxSetDoubleBuffering(gfxScreen_t screen, bool enable);

///@}

///@name Rendering and presentation
///@{

/**
 * @brief Retrieves the framebuffer of the specified screen to which graphics should be rendered.
 * @param screen Screen ID (see \ref gfxScreen_t)
 * @param side Framebuffer side (see \ref gfx3dSide_t) (pass \ref GFX_LEFT if not using stereoscopic 3D)
 * @param width Pointer that will hold the width of the framebuffer in pixels.
 * @param height Pointer that will hold the height of the framebuffer in pixels.
 * @return A pointer to the current framebuffer of the chosen screen.
 *
 * Please remember that the returned pointer will change every frame if double buffering is enabled.
 */
u8* gfxGetFramebuffer(gfxScreen_t screen, gfx3dSide_t side, u16* width, u16* height);

/**
 * @brief Flushes the data cache for the current framebuffers.
 * @warning This is **only used during software rendering**. Since this function has significant overhead,
 *          it is preferred to call this only once per frame, after all software rendering is completed.
 */
void gfxFlushBuffers(void);

/**
 * @brief Updates the configuration of the specified screen, swapping the buffers if double buffering is enabled.
 * @param scr Screen ID (see \ref gfxScreen_t)
 * @param hasStereo For the top screen in 3D mode: true if the framebuffer contains individual images
 *                  for both eyes, or false if the left image should be duplicated to the right eye.
 * @note Previously rendered content will be displayed on the screen after the next VBlank.
 * @note This function is still useful even if double buffering is disabled, as it must be used to commit configuration changes.
 * @warning Only call this once per screen per frame, otherwise graphical glitches will occur
 *          since this API does not implement triple buffering.
 */
void gfxScreenSwapBuffers(gfxScreen_t scr, bool hasStereo);

/**
 * @brief Same as \ref gfxScreenSwapBuffers, but with hasStereo set to true.
 * @param scr Screen ID (see \ref gfxScreen_t)
 * @param immediate This parameter no longer has any effect and is thus ignored.
 * @deprecated This function has been superseded by \ref gfxScreenSwapBuffers, please use that instead.
 */
DEPRECATED void gfxConfigScreen(gfxScreen_t scr, bool immediate);

/**
 * @brief Updates the configuration of both screens.
 * @note This function is equivalent to: \code gfxScreenSwapBuffers(GFX_TOP,true); gfxScreenSwapBuffers(GFX_BOTTOM,true); \endcode
 */
void gfxSwapBuffers(void);

/// Same as \ref gfxSwapBuffers (formerly different).
void gfxSwapBuffersGpu(void);

///@}
