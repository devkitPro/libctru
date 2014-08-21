#ifndef OS_H
#define OS_H

u32 osConvertVirtToPhys(u32 vaddr);
const char* osStrError(u32 error);

#endif
