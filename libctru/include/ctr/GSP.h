#ifndef GSP_H
#define GSP_H

void GSPGPU_AcquireRight(Handle handle, u8 flags);
void GSPGPU_SetLcdForceBlack(Handle handle, u8 flags);
void GSPGPU_WriteHWRegs(Handle handle, u32 regAddr, u8* data, u8 size);
void GSPGPU_ReadHWRegs(Handle handle, u32 regAddr, u8* data, u8 size);
void GSPGPU_RegisterInterruptRelayQueue(Handle handle, Handle eventHandle, u32 flags, Handle* outMemHandle, u8* threadID);
Result GSPGPU_TriggerCmdReqQueue(Handle handle);
Result GSPGPU_submitGxCommand(u32* sharedGspCmdBuf, u32 gxCommand[0x20], Handle handle);

#endif
