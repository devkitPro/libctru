#ifndef HB_H
#define HB_H

// WARNING ! THIS FILE PROVIDES AN INTERFACE TO A NON-OFFICIAL SERVICE PROVIDED BY NINJHAX
// BY USING COMMANDS FROM THIS SERVICE YOU WILL LIKELY MAKE YOUR APPLICATION INCOMPATIBLE WITH OTHER HOMEBREW LAUNCHING METHODS
// A GOOD WAY TO COPE WITH THIS IS TO CHECK THE OUTPUT OF initHb FOR ERRORS

#include <3ds/types.h>

Result initHb();
void exitHb();

Result HB_GetBootloaderAddresses(void** load3dsx, void** setArgv);
Result HB_ReprotectMemory(u32* addr, u32 pages, u32 mode, u32* reprotectedPages);

#endif
