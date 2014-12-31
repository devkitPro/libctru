#ifndef HB_H
#define HB_H

// WARNING ! THIS FILE PROVIDES AN INTERFACE TO A NON-OFFICIAL SERVICE PROVIDED BY NINJHAX
// BY USING COMMANDS FROM THIS SERVICE YOU WILL LIKELY MAKE YOUR APPLICATION INCOMPATIBLE WITH OTHER HOMEBREW LAUNCHING METHODS
// A GOOD WAY TO COPE WITH THIS IS TO CHECK THE OUTPUT OF initHb FOR ERRORS

#include <3ds/types.h>

Result hbInit();
void hbExit();

// flushes/invalidates entire data/instruction cache
// can be useful when writing code to executable pages
Result HB_FlushInvalidateCache(void);

// fetches the address for ninjhax bootloader addresses, useful for running 3dsx executables
// void (*callBootloader)(Handle hb, Handle file);
// void (*setArgs)(u32* src, u32 length);
Result HB_GetBootloaderAddresses(void** load3dsx, void** setArgv);

// changes the permissions of a given number of pages at address addr to mode
// should it fail, the appropriate kernel error code will be returned and *reprotectedPages (if not NULL)
// will be set to the number of sequential pages which were successfully reprotected + 1
Result HB_ReprotectMemory(u32* addr, u32 pages, u32 mode, u32* reprotectedPages);

#endif
