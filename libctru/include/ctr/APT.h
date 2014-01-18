#ifndef APT_H
#define APT_H

void APT_GetLockHandle(Handle handle, u16 flags, Handle* lockHandle);
void APT_Initialize(Handle handle, u32 a, Handle* eventHandle1, Handle* eventHandle2);
Result APT_Enable(Handle handle, u32 a);
Result APT_PrepareToJumpToHomeMenu(Handle handle);
Result APT_JumpToHomeMenu(Handle handle, u32 a, u32 b, u32 c);

#endif
