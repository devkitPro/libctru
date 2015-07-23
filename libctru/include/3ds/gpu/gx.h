/**
 * @file gx.h
 */

#pragma once

#define GX_BUFFER_DIM(w, h) (((h)<<16)|((w)&0xFFFF))

/**
 * @brief Pixel formats
 * @sa GSP_FramebufferFormats
 */
typedef enum
{
	GX_TRANSFER_FMT_RGBA8  = 0,
	GX_TRANSFER_FMT_RGB8   = 1,
	GX_TRANSFER_FMT_RGB565 = 2,
	GX_TRANSFER_FMT_RGB5A1 = 3,
	GX_TRANSFER_FMT_RGBA4  = 4
} GX_TRANSFER_FORMAT;

/**
 * @brief Anti-aliasing modes
 *
 * Please remember that the framebuffer is sideways.
 * Hence if you activate 2x1 anti-aliasing the destination dimensions are w = 240*2 and h = 400
 */
typedef enum
{
	GX_TRANSFER_SCALE_NO = 0, ///< No anti-aliasing
	GX_TRANSFER_SCALE_X  = 1, ///< 2x1 anti-aliasing
	GX_TRANSFER_SCALE_XY = 2, ///< 2x2 anti-aliasing
} GX_TRANSFER_SCALE;

/**
 * @brief GX transfer control flags
 */
typedef enum
{
	GX_FILL_TRIGGER     = 0x001, ///< Trigger the PPF event
	GX_FILL_FINISHED    = 0x002, ///< Indicates if the memory fill is complete. You should not use it when requesting a transfer.
	GX_FILL_16BIT_DEPTH = 0x000, ///< The buffer has a 16 bit per pixel depth
	GX_FILL_24BIT_DEPTH = 0x100, ///< The buffer has a 24 bit per pixel depth
	GX_FILL_32BIT_DEPTH = 0x200, ///< The buffer has a 32 bit per pixel depth
} GX_FILL_CONTROL;

#define GX_TRANSFER_FLIP_VERT(x)  ((x)<<0)
#define GX_TRANSFER_OUT_TILED(x)  ((x)<<1)
#define GX_TRANSFER_RAW_COPY(x)   ((x)<<3)
#define GX_TRANSFER_IN_FORMAT(x)  ((x)<<8)
#define GX_TRANSFER_OUT_FORMAT(x) ((x)<<12)
#define GX_TRANSFER_SCALING(x)    ((x)<<24)

Result GX_RequestDma(u32* gxbuf, u32* src, u32* dst, u32 length);
Result GX_SetCommandList_Last(u32* gxbuf, u32* buf0a, u32 buf0s, u8 flags);
Result GX_SetMemoryFill(u32* gxbuf, u32* buf0a, u32 buf0v, u32* buf0e, u16 control0, u32* buf1a, u32 buf1v, u32* buf1e, u16 control1);
Result GX_SetDisplayTransfer(u32* gxbuf, u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 flags);
Result GX_SetTextureCopy(u32* gxbuf, u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 size, u32 flags);
Result GX_SetCommandList_First(u32* gxbuf, u32* buf0a, u32 buf0s, u32* buf1a, u32 buf1s, u32* buf2a, u32 buf2s);
