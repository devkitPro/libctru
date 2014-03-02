#ifndef GSP_H
#define GSP_H

#define GSP_REBASE_REG(r) ((r)-0x1EB00000)

void gspInit();
void gspExit();

Result GSPGPU_AcquireRight(Handle *handle, u8 flags);
Result GSPGPU_ReleaseRight(Handle *handle);
Result GSPGPU_SetLcdForceBlack(Handle *handle, u8 flags);
Result GSPGPU_FlushDataCache(Handle *handle, u8* adr, u32 size);
Result GSPGPU_WriteHWRegs(Handle *handle, u32 regAddr, u32* data, u8 size);
Result GSPGPU_WriteHWRegsWithMask(Handle* handle, u32 regAddr, u32* data, u8 datasize, u32* maskdata, u8 masksize);
Result GSPGPU_ReadHWRegs(Handle *handle, u32 regAddr, u32* data, u8 size);
Result GSPGPU_RegisterInterruptRelayQueue(Handle *handle, Handle eventHandle, u32 flags, Handle* outMemHandle, u8* threadID);
Result GSPGPU_UnregisterInterruptRelayQueue(Handle* handle);
Result GSPGPU_TriggerCmdReqQueue(Handle *handle);
Result GSPGPU_submitGxCommand(u32* sharedGspCmdBuf, u32 gxCommand[0x8], Handle* handle);

#endif
