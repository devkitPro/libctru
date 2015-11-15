/**
 * @file hb.h
 * @brief HB (Homebrew) service.
 */
#pragma once

// WARNING ! THIS FILE PROVIDES AN INTERFACE TO A NON-OFFICIAL SERVICE PROVIDED BY NINJHAX
// BY USING COMMANDS FROM THIS SERVICE YOU WILL LIKELY MAKE YOUR APPLICATION INCOMPATIBLE WITH OTHER HOMEBREW LAUNCHING METHODS
// A GOOD WAY TO COPE WITH THIS IS TO CHECK THE OUTPUT OF hbInit FOR ERRORS

#include <3ds/types.h>

/**
 * @brief Initializes HB.
 * @deprecated Only available with the outdated Ninjhax 1.1.
 */
Result hbInit(void) DEPRECATED;

/**
 * @brief Exits HB.
 * @deprecated Only available with the outdated Ninjhax 1.1.
 */
void hbExit(void) DEPRECATED;

/**
 * @brief Flushes/invalidates the entire data/instruction cache.
 * @deprecated Only available with the outdated Ninjhax 1.1.
 */
Result HB_FlushInvalidateCache(void) DEPRECATED;

/**
 * @brief Fetches the address for Ninjhax 1.x bootloader addresses.
 * @param load3dsx void (*callBootloader)(Handle hb, Handle file);
 * @param setArgv void (*setArgs)(u32* src, u32 length);
 * @deprecated Only available with the outdated Ninjhax 1.1.
 */
Result HB_GetBootloaderAddresses(void** load3dsx, void** setArgv) DEPRECATED;

/**
 * @brief Changes the permissions of a given number of pages at address addr to mode.
 * Should it fail, the appropriate kernel error code will be returned and *reprotectedPages (if not NULL)
 * will be set to the number of sequential pages which were successfully reprotected + 1
 * @param addr Address to reprotect.
 * @param pages Number of pages to reprotect.
 * @param mode Mode to reprotect to.
 * @param reprotectedPages Number of successfully reprotected pages, on failure.
 * @deprecated Only available with the outdated Ninjhax 1.1.
 */
Result HB_ReprotectMemory(u32* addr, u32 pages, u32 mode, u32* reprotectedPages) DEPRECATED;
