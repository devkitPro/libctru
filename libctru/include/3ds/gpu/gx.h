/**
 * @file gx.h
 * @brief GX commands.
 */
#pragma once

/**
 * @brief Creates a buffer dimension parameter from width and height values.
 * @param w buffer width for GX_DisplayTransfer, linesize for GX_TextureCopy
 * @param h buffer height for GX_DisplayTransfer, gap for GX_TextureCopy
 */
#define GX_BUFFER_DIM(w, h) (((h)<<16)|((w)&0xFFFF))

/**
 * @brief Supported transfer pixel formats.
 * @sa GSPGPU_FramebufferFormats
 */
typedef enum
{
	GX_TRANSFER_FMT_RGBA8  = 0, ///< 8-bit Red + 8-bit Green + 8-bit Blue + 8-bit Alpha
	GX_TRANSFER_FMT_RGB8   = 1, ///< 8-bit Red + 8-bit Green + 8-bit Blue
	GX_TRANSFER_FMT_RGB565 = 2, ///< 5-bit Red + 6-bit Green + 5-bit Blue
	GX_TRANSFER_FMT_RGB5A1 = 3, ///< 5-bit Red + 5-bit Green + 5-bit Blue + 1-bit Alpha
	GX_TRANSFER_FMT_RGBA4  = 4  ///< 4-bit Red + 4-bit Green + 4-bit Blue + 4-bit Alpha
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

/// GX transfer control flags
typedef enum
{
	GX_FILL_TRIGGER     = 0x001, ///< Trigger the PPF event
	GX_FILL_FINISHED    = 0x002, ///< Indicates if the memory fill is complete. You should not use it when requesting a transfer.
	GX_FILL_16BIT_DEPTH = 0x000, ///< The buffer has a 16 bit per pixel depth
	GX_FILL_24BIT_DEPTH = 0x100, ///< The buffer has a 24 bit per pixel depth
	GX_FILL_32BIT_DEPTH = 0x200, ///< The buffer has a 32 bit per pixel depth
} GX_FILL_CONTROL;

/// Creates a transfer vertical flip flag.
#define GX_TRANSFER_FLIP_VERT(x)  ((x)<<0)
/// Creates a transfer tiled output flag.
#define GX_TRANSFER_OUT_TILED(x)  ((x)<<1)
/// Creates a transfer raw copy flag.
#define GX_TRANSFER_RAW_COPY(x)   ((x)<<3)
/// Creates a transfer input format flag.
#define GX_TRANSFER_IN_FORMAT(x)  ((x)<<8)
/// Creates a transfer output format flag.
#define GX_TRANSFER_OUT_FORMAT(x) ((x)<<12)
/// Creates a transfer scaling flag.
#define GX_TRANSFER_SCALING(x)    ((x)<<24)

/// Command list flag bit 0.
#define GX_CMDLIST_BIT0  BIT(0)
/// Flushes the command list.
#define GX_CMDLIST_FLUSH BIT(1)

extern u32* gxCmdBuf; ///< GX command buffer.

/**
 * @brief Requests a DMA.
 * @param src Source to DMA from.
 * @param dst Destination to DMA to.
 * @param length Length of data to transfer.
 */
Result GX_RequestDma(u32* src, u32* dst, u32 length);

/**
 * @brief Processes a GPU command list.
 * @param buf0a Command list address.
 * @param buf0s Command list size.
 * @param flags Flags to process with.
 */
Result GX_ProcessCommandList(u32* buf0a, u32 buf0s, u8 flags);

/**
 * @brief Fills the memory of two buffers with the given values.
 * @param buf0a Start address of the first buffer.
 * @param buf0v Dimensions of the first buffer.
 * @param buf0e End address of the first buffer.
 * @param control0 Value to fill the first buffer with.
 * @param buf1a Start address of the second buffer.
 * @param buf1v Dimensions of the second buffer.
 * @param buf1e End address of the second buffer.
 * @param control1 Value to fill the second buffer with.
 */
Result GX_MemoryFill(u32* buf0a, u32 buf0v, u32* buf0e, u16 control0, u32* buf1a, u32 buf1v, u32* buf1e, u16 control1);

/**
 * @brief Initiates a display transfer.
 * @note The PPF event will be signaled on completion.
 * @param inadr Address of the input.
 * @param indim Dimensions of the input.
 * @param outadr Address of the output.
 * @param outdim Dimensions of the output.
 * @param flags Flags to transfer with.
 */
Result GX_DisplayTransfer(u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 flags);

/**
 * @brief Initiates a texture copy.
 * @note The PPF event will be signaled on completion.
 * @param inadr Address of the input.
 * @param indim Dimensions of the input.
 * @param outadr Address of the output.
 * @param outdim Dimensions of the output.
 * @param size Size of the data to transfer.
 * @param flags Flags to transfer with.
 */
Result GX_TextureCopy(u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 size, u32 flags);

/**
 * @brief Flushes the cache regions of three buffers.
 * @param buf0a Address of the first buffer.
 * @param buf0s Size of the first buffer.
 * @param buf1a Address of the second buffer.
 * @param buf1s Size of the second buffer.
 * @param buf2a Address of the third buffer.
 * @param buf2s Size of the third buffer.
 */
Result GX_FlushCacheRegions(u32* buf0a, u32 buf0s, u32* buf1a, u32 buf1s, u32* buf2a, u32 buf2s);
