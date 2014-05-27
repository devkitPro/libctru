#ifndef GSP_H
#define GSP_H

#define GSP_REBASE_REG(r) ((r)-0x1EB00000)

typedef struct
{
	u32 active_framebuf;//"0=first, 1=second"
	u32 *framebuf0_vaddr;//"Framebuffer virtual address, for the main screen this is the 3D left framebuffer"
	u32 *framebuf1_vaddr;//"For the main screen: 3D right framebuffer address"
	u32 framebuf_widthbytesize;//"Value for 0x1EF00X90, controls framebuffer width"
	u32 format;//"Framebuffer format, this u16 is written to the low u16 for LCD register 0x1EF00X70."
	u32 framebuf_dispselect;//"Value for 0x1EF00X78, controls which framebuffer is displayed"
	u32 unk;//"?"
} GSP_FramebufferInfo;

typedef struct//See this for GSP_CaptureInfoEntry and GSP_CaptureInfo: http://3dbrew.org/wiki/GSPGPU:ImportDisplayCaptureInfo
{
	u32 *framebuf0_vaddr;
	u32 *framebuf1_vaddr;
	u32 format;
	u32 framebuf_widthbytesize;
} GSP_CaptureInfoEntry;

typedef struct
{
	GSP_CaptureInfoEntry screencapture[2];
} GSP_CaptureInfo;

Result gspInit();
void gspExit();

Result GSPGPU_AcquireRight(Handle *handle, u8 flags);
Result GSPGPU_ReleaseRight(Handle *handle);
Result GSPGPU_ImportDisplayCaptureInfo(Handle* handle, GSP_CaptureInfo *captureinfo);
Result GSPGPU_SaveVramSysArea(Handle* handle);
Result GSPGPU_RestoreVramSysArea(Handle* handle);
Result GSPGPU_SetLcdForceBlack(Handle *handle, u8 flags);
Result GSPGPU_SetBufferSwap(Handle* handle, u32 screenid, GSP_FramebufferInfo *framebufinfo);
Result GSPGPU_FlushDataCache(Handle *handle, u8* adr, u32 size);
Result GSPGPU_InvalidateDataCache(Handle* handle, u8* adr, u32 size);
Result GSPGPU_WriteHWRegs(Handle *handle, u32 regAddr, u32* data, u8 size);
Result GSPGPU_WriteHWRegsWithMask(Handle* handle, u32 regAddr, u32* data, u8 datasize, u32* maskdata, u8 masksize);
Result GSPGPU_ReadHWRegs(Handle *handle, u32 regAddr, u32* data, u8 size);
Result GSPGPU_RegisterInterruptRelayQueue(Handle *handle, Handle eventHandle, u32 flags, Handle* outMemHandle, u8* threadID);
Result GSPGPU_UnregisterInterruptRelayQueue(Handle* handle);
Result GSPGPU_TriggerCmdReqQueue(Handle *handle);

Result GSPGPU_submitGxCommand(u32* sharedGspCmdBuf, u32 gxCommand[0x8], Handle* handle);

#endif
