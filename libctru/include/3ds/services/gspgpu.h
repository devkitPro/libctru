/**
 * @file gspgpu.h
 * @brief GSPGPU service.
 */
#pragma once

#define GSP_SCREEN_TOP           0   ///< ID of the top screen.
#define GSP_SCREEN_BOTTOM        1   ///< ID of the bottom screen.
#define GSP_SCREEN_WIDTH         240 ///< Width of the top/bottom screens.
#define GSP_SCREEN_HEIGHT_TOP    400 ///< Height of the top screen.
#define GSP_SCREEN_HEIGHT_TOP_2X 800 ///< Height of the top screen (2x).
#define GSP_SCREEN_HEIGHT_BOTTOM 320 ///< Height of the bottom screen.

/// Framebuffer information.
typedef struct
{
	u32 active_framebuf;        ///< Active framebuffer. (0 = first, 1 = second)
	u32 *framebuf0_vaddr;       ///< Framebuffer virtual address, for the main screen this is the 3D left framebuffer.
	u32 *framebuf1_vaddr;       ///< For the main screen: 3D right framebuffer address.
	u32 framebuf_widthbytesize; ///< Value for 0x1EF00X90, controls framebuffer width.
	u32 format;                 ///< Framebuffer format, this u16 is written to the low u16 for LCD register 0x1EF00X70.
	u32 framebuf_dispselect;    ///< Value for 0x1EF00X78, controls which framebuffer is displayed.
	u32 unk;                    ///< Unknown.
} GSPGPU_FramebufferInfo;

/// Framebuffer format.
typedef enum
{
	GSP_RGBA8_OES=0,   ///< RGBA8. (4 bytes)
	GSP_BGR8_OES=1,    ///< BGR8. (3 bytes)
	GSP_RGB565_OES=2,  ///< RGB565. (2 bytes)
	GSP_RGB5_A1_OES=3, ///< RGB5A1. (2 bytes)
	GSP_RGBA4_OES=4    ///< RGBA4. (2 bytes)
} GSPGPU_FramebufferFormat;

/// Capture info entry.
typedef struct
{
	u32 *framebuf0_vaddr;       ///< Left framebuffer.
	u32 *framebuf1_vaddr;       ///< Right framebuffer.
	u32 format;                 ///< Framebuffer format.
	u32 framebuf_widthbytesize; ///< Framebuffer pitch.
} GSPGPU_CaptureInfoEntry;

/// Capture info.
typedef struct
{
	GSPGPU_CaptureInfoEntry screencapture[2]; ///< Capture info entries, one for each screen.
} GSPGPU_CaptureInfo;

/// GSPGPU events.
typedef enum
{
	GSPGPU_EVENT_PSC0 = 0, ///< Memory fill completed.
	GSPGPU_EVENT_PSC1,     ///< TODO
	GSPGPU_EVENT_VBlank0,  ///< TODO
	GSPGPU_EVENT_VBlank1,  ///< TODO
	GSPGPU_EVENT_PPF,      ///< Display transfer finished.
	GSPGPU_EVENT_P3D,      ///< Command list processing finished.
	GSPGPU_EVENT_DMA,      ///< TODO

	GSPGPU_EVENT_MAX,      ///< Used to know how many events there are.
} GSPGPU_Event;

/**
 * @brief Gets the number of bytes per pixel for the specified format.
 * @param format See \ref GSPGPU_FramebufferFormat.
 * @return Bytes per pixel.
 */
static inline unsigned gspGetBytesPerPixel(GSPGPU_FramebufferFormat format)
{
	switch (format)
	{
		case GSP_RGBA8_OES:
			return 4;
		default:
		case GSP_BGR8_OES:
			return 3;
		case GSP_RGB565_OES:
		case GSP_RGB5_A1_OES:
		case GSP_RGBA4_OES:
			return 2;
	}
}

/// Initializes GSPGPU.
Result gspInit(void);

/// Exits GSPGPU.
void gspExit(void);

/// Returns true if the application currently has GPU rights.
bool gspHasGpuRight(void);

/**
 * @brief Presents a buffer to the specified screen.
 * @param screen Screen ID (see \ref GSP_SCREEN_TOP and \ref GSP_SCREEN_BOTTOM)
 * @param swap Specifies which set of framebuffer registers to configure and activate (0 or 1)
 * @param fb_a Pointer to the framebuffer (in stereo mode: left eye)
 * @param fb_b Pointer to the secondary framebuffer (only used in stereo mode for the right eye, otherwise pass the same as fb_a)
 * @param stride Stride in bytes between scanlines
 * @param mode Mode configuration to be written to LCD register
 * @note The most recently presented buffer is processed and configured during the specified screen's next VBlank event.
 */
void gspPresentBuffer(unsigned screen, unsigned swap, const void* fb_a, const void* fb_b, u32 stride, u32 mode);

/**
 * @brief Configures a callback to run when a GSPGPU event occurs.
 * @param id ID of the event.
 * @param cb Callback to run.
 * @param data Data to be passed to the callback.
 * @param oneShot When true, the callback is only executed once. When false, the callback is executed every time the event occurs.
 */
void gspSetEventCallback(GSPGPU_Event id, ThreadFunc cb, void* data, bool oneShot);

/**
 * @brief Waits for a GSPGPU event to occur.
 * @param id ID of the event.
 * @param nextEvent Whether to discard the current event and wait for the next event.
 */
void gspWaitForEvent(GSPGPU_Event id, bool nextEvent);

/**
 * @brief Waits for any GSPGPU event to occur.
 * @return The ID of the event that occurred.
 *
 * The function returns immediately if there are unprocessed events at the time of call.
 */
GSPGPU_Event gspWaitForAnyEvent(void);

/// Waits for PSC0
#define gspWaitForPSC0() gspWaitForEvent(GSPGPU_EVENT_PSC0, false)

/// Waits for PSC1
#define gspWaitForPSC1() gspWaitForEvent(GSPGPU_EVENT_PSC1, false)

/// Waits for VBlank.
#define gspWaitForVBlank() gspWaitForVBlank0()

/// Waits for VBlank0.
#define gspWaitForVBlank0() gspWaitForEvent(GSPGPU_EVENT_VBlank0, true)

/// Waits for VBlank1.
#define gspWaitForVBlank1() gspWaitForEvent(GSPGPU_EVENT_VBlank1, true)

/// Waits for PPF.
#define gspWaitForPPF() gspWaitForEvent(GSPGPU_EVENT_PPF, false)

/// Waits for P3D.
#define gspWaitForP3D() gspWaitForEvent(GSPGPU_EVENT_P3D, false)

/// Waits for DMA.
#define gspWaitForDMA() gspWaitForEvent(GSPGPU_EVENT_DMA, false)

/**
 * @brief Submits a GX command.
 * @param gxCommand GX command to execute.
 */
Result gspSubmitGxCommand(u32 gxCommand[0x8]);

/**
 * @brief Acquires GPU rights.
 * @param flags Flags to acquire with.
 */
Result GSPGPU_AcquireRight(u8 flags);

/// Releases GPU rights.
Result GSPGPU_ReleaseRight(void);

/**
 * @brief Retrieves display capture info.
 * @param captureinfo Pointer to output capture info to.
 */
Result GSPGPU_ImportDisplayCaptureInfo(GSPGPU_CaptureInfo*captureinfo);

/// Saves the VRAM sys area.
Result GSPGPU_SaveVramSysArea(void);

/// Resets the GPU
Result GSPGPU_ResetGpuCore(void);

/// Restores the VRAM sys area.
Result GSPGPU_RestoreVramSysArea(void);

/**
 * @brief Sets whether to force the LCD to black.
 * @param flags Whether to force the LCD to black. (0 = no, non-zero = yes)
 */
Result GSPGPU_SetLcdForceBlack(u8 flags);

/**
 * @brief Updates a screen's framebuffer state.
 * @param screenid ID of the screen to update.
 * @param framebufinfo Framebuffer information to update with.
 */
Result GSPGPU_SetBufferSwap(u32 screenid, GSPGPU_FramebufferInfo*framebufinfo);

/**
 * @brief Flushes memory from the data cache.
 * @param adr Address to flush.
 * @param size Size of the memory to flush.
 */
Result GSPGPU_FlushDataCache(const void* adr, u32 size);

/**
 * @brief Invalidates memory in the data cache.
 * @param adr Address to invalidate.
 * @param size Size of the memory to invalidate.
 */
Result GSPGPU_InvalidateDataCache(const void* adr, u32 size);

/**
 * @brief Writes to GPU hardware registers.
 * @param regAddr Register address to write to.
 * @param data Data to write.
 * @param size Size of the data to write.
 */
Result GSPGPU_WriteHWRegs(u32 regAddr, u32* data, u8 size);

/**
 * @brief Writes to GPU hardware registers with a mask.
 * @param regAddr Register address to write to.
 * @param data Data to write.
 * @param datasize Size of the data to write.
 * @param maskdata Data of the mask.
 * @param masksize Size of the mask.
 */
Result GSPGPU_WriteHWRegsWithMask(u32 regAddr, u32* data, u8 datasize, u32* maskdata, u8 masksize);

/**
 * @brief Reads from GPU hardware registers.
 * @param regAddr Register address to read from.
 * @param data Buffer to read data to.
 * @param size Size of the buffer.
 */
Result GSPGPU_ReadHWRegs(u32 regAddr, u32* data, u8 size);

/**
 * @brief Registers the interrupt relay queue.
 * @param eventHandle Handle of the GX command event.
 * @param flags Flags to register with.
 * @param outMemHandle Pointer to output the shared memory handle to.
 * @param threadID Pointer to output the GSP thread ID to.
 */
Result GSPGPU_RegisterInterruptRelayQueue(Handle eventHandle, u32 flags, Handle* outMemHandle, u8* threadID);

/// Unregisters the interrupt relay queue.
Result GSPGPU_UnregisterInterruptRelayQueue(void);

/// Triggers a handling of commands written to shared memory.
Result GSPGPU_TriggerCmdReqQueue(void);

/**
 * @brief Sets 3D_LEDSTATE to the input state value.
 * @param disable False = 3D LED enable, true = 3D LED disable.
 */
Result GSPGPU_SetLedForceOff(bool disable);
