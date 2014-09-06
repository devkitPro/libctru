#pragma once

#define SYSTEM_VERSION(major, minor, revision) \
	(((major)<<24)|((minor)<<16)|((revision)<<8))

u32 osConvertVirtToPhys(u32 vaddr);
const char* osStrError(u32 error);
u32 osGetFirmVersion();
u32 osGetKernelVersion();
u64 osGetTime();
