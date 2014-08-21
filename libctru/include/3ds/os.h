#ifndef OS_H
#define OS_H

typedef struct {
    u8 major, minor, revision;
} sysVersion;

u32 osConvertVirtToPhys(u32 vaddr);
const char* osStrError(u32 error);
sysVersion osGetFirmVersion();
sysVersion osGetKernelVersion();

#endif
