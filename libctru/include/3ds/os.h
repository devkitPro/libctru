/**
 * @file os.h
 * @brief OS related stuff.
 */
#pragma once

/// Packs a system version from its components.
#define SYSTEM_VERSION(major, minor, revision) \
	(((major)<<24)|((minor)<<16)|((revision)<<8))

/// Retrieves the major version from a packed system version.
#define GET_VERSION_MAJOR(version)    ((version) >>24)

/// Retrieves the minor version from a packed system version.
#define GET_VERSION_MINOR(version)    (((version)>>16)&0xFF)

/// Retrieves the revision version from a packed system version.
#define GET_VERSION_REVISION(version) (((version)>> 8)&0xFF)

/**
 * @brief Converts an address from virtual (process) memory to physical memory.
 * @param vaddr Input virtual address.
 * @return The corresponding physical address.
 * It is sometimes required by services or when using the GPU command buffer.
 */
u32 osConvertVirtToPhys(u32 vaddr);

/**
 * @brief Converts 0x14* vmem to 0x30*.
 * @param addr Input address.
 * @return The corresponding address in the 0x30* range, the input address if it's already within the new vmem, or 0 if it's outside of both ranges.
 */
u32 osConvertOldLINEARMemToNew(u32 addr);

/**
 * @brief Retrieves basic information about a service error.
 * @param error Error to retrieve information about.
 * @return A string containing a summary of an error.
 *
 * This can be used to get some details about an error returned by a service call.
 */
const char* osStrError(u32 error);

/**
 * @brief Gets the system's FIRM version.
 * @return The system's FIRM version.
 *
 * This can be used to compare system versions easily with @ref SYSTEM_VERSION.
 */
u32 osGetFirmVersion(void);

/**
 * @brief Gets the system's kernel version.
 * @return The system's kernel version.
 *
 * This can be used to compare system versions easily with @ref SYSTEM_VERSION.
 *
 * @code
 * if(osGetKernelVersion() > SYSTEM_VERSION(2,46,0)) printf("You are running 9.0 or higher\n");
 * @endcode
 */
u32 osGetKernelVersion(void);

/**
 * @brief Gets the current time.
 * @return The number of milliseconds since 1st Jan 1900 00:00.
 */
u64 osGetTime(void);

/**
 * @brief Gets the current Wifi signal strength.
 * @return The current Wifi signal strength.
 *
 * Valid values are 0-3:
 * - 0 means the singal strength is terrible or the 3DS is disconnected from
 *   all networks.
 * - 1 means the signal strength is bad.
 * - 2 means the signal strength is decent.
 * - 3 means the signal strength is good.
 *
 * Values outside the range of 0-3 should never be returned.
 *
 * These values correspond with the number of wifi bars displayed by Home Menu.
 */
u8 osGetWifiStrength(void);
