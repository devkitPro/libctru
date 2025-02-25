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
 * GSPGPU performance log entry.
 *
 * Use the lastDurationUs field when benchmarking single GPU operations, this is usally meant
 * for 3D library writers.
 *
 * Use the difference between two totalDurationUs when using a GPU library (e.g. citro3d), as
 * there can be multiple GPU operations (e.g. P3D, PPF) per render pass, or per frame, and so on.
 * Don't use totalDurationUs as-is (rather, take the difference as just described), because it
 * can overflow.
 */
typedef struct
{
	u32 lastDurationUs;    ///< Duration of the last corresponding PICA200 operation (time between op is started and IRQ is received).
	u32 totalDurationUs;   ///< Sum of lastDurationUs for the corresponding PICA200 operation. Can overflow.
} GSPGPU_PerfLogEntry;

/// GSPGPU performance log
typedef struct
{
	GSPGPU_PerfLogEntry entries[GSPGPU_EVENT_MAX]; ///< Performance log entries (one per operation/"event").
} GSPGPU_PerfLog;

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

/**
 * @brief Gets a pointer to the current gsp::Gpu session handle.
 * @return A pointer to the current gsp::Gpu session handle.
 */
Handle *gspGetSessionHandle(void);

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
 * @return true if a buffer had already been presented to the screen but not processed yet by GSP, false otherwise.
 * @note The most recently presented buffer is processed and configured during the specified screen's next VBlank event.
 */
bool gspPresentBuffer(unsigned screen, unsigned swap, const void* fb_a, const void* fb_b, u32 stride, u32 mode);

/**
 * @brief Returns true if a prior \ref gspPresentBuffer command is still pending to be processed by GSP.
 * @param screen Screen ID (see \ref GSP_SCREEN_TOP and \ref GSP_SCREEN_BOTTOM)
 */
bool gspIsPresentPending(unsigned screen);

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
Result gspSubmitGxCommand(const u32 gxCommand[0x8]);

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
Result GSPGPU_ImportDisplayCaptureInfo(GSPGPU_CaptureInfo* captureinfo);

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
Result GSPGPU_SetBufferSwap(u32 screenid, const GSPGPU_FramebufferInfo* framebufinfo);

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
Result GSPGPU_WriteHWRegs(u32 regAddr, const u32* data, u8 size);

/**
 * @brief Writes to GPU hardware registers with a mask.
 * @param regAddr Register address to write to.
 * @param data Data to write.
 * @param datasize Size of the data to write.
 * @param maskdata Data of the mask.
 * @param masksize Size of the mask.
 */
Result GSPGPU_WriteHWRegsWithMask(u32 regAddr, const u32* data, u8 datasize, const u32* maskdata, u8 masksize);

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

/**
 * @brief Enables or disables the performance log and clear
 *        its state to zero.
 * @param enabled Whether to enable the performance log.
 * @note  It is assumed that no GPU operation is in progress when calling this function.
 * @bug   The official sysmodule forgets to clear the "start tick" states to 0, though
 *        this should not be much of an issue (as per the note above).
 */
Result GSPGPU_SetPerfLogMode(bool enabled);

/**
 * @brief Retrieves the performance log.
 * @param[out] outPerfLog Pointer to output the performance log to.
 * @note  Use the difference between two totalDurationUs when using a GPU library (e.g. citro3d), as
 *        there can be multiple GPU operations (e.g. P3D, PPF) per render pass, or per frame, and so on.
 *        Don't use totalDurationUs as-is (rather, take the difference as just described), because it
 *        can overflow.
 * @note  For a MemoryFill operation that uses both PSC0 and PSC1, take the maximum
 *        of the two "last duration" entries.
 * @note  For PDC0/PDC1 (VBlank0/1), the "last duration" entry corresponds to the time between
 *        the current PDC (VBlank) IRQ and the previous one. The official GSP sysmodule
 *        assumes both PDC0 and PDC1 IRQ happens at the same rate (this is almost always
 *        the case, but not always if user changes PDC timings), and sets both entries
 *        in the PDC0 handler.
 * @bug   The official sysmodule doesn't handle the PDC0/1 entries correctly after init. On the first
 *        frame \ref GSPGPU_SetPerfLogMode is enabled, "last duration" will have a nonsensical
 *        value; and "total duration" stays nonsensical. This isn't much of a problem, except for the
 *        first frame, because "total duration" is not supposed to be used as-is (you are supposed
 *        to take the difference of this field between two time points of your choosing, instead).
 * @bug   Since it is running at approx. 3.25 GiB/s per bank, some small PSC operations might
 *        complete before the official GSP has time to record the start time.
 * @bug   The official sysmodule doesn't properly handle data synchronization for the perflog,
 *        in practice this should be fine, however.
 */
Result GSPGPU_GetPerfLog(GSPGPU_PerfLog *outPerfLog);
