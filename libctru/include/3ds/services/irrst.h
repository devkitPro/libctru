#pragma once

//See also: http://3dbrew.org/wiki/IR_Services http://3dbrew.org/wiki/IRRST_Shared_Memory

#include "3ds/services/hid.h" // for circlePosition definition

#define IRRST_SHAREDMEM_DEFAULT (0x1000A000)

extern Handle irrstMemHandle;
extern vu32* irrstSharedMem;

Result irrstInit(u32* sharedMem);
void irrstExit();

void irrstScanInput();
u32 irrstKeysHeld();
void irrstCstickRead(circlePosition* pos);

void irrstWaitForEvent(bool nextEvent);

#define hidCstickRead irrstCstickRead // because why not

Result IRRST_GetHandles(Handle* outMemHandle, Handle* outEventHandle);
Result IRRST_Initialize(u32 unk1, u8 unk2);
Result IRRST_Shutdown(void);
