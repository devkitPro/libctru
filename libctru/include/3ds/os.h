/**
 * @file os.h
 * @brief OS related stuff.
 */
#pragma once
#include "svc.h"

/// Packs a system version from its components.
#define SYSTEM_VERSION(major, minor, revision) \
	(((major)<<24)|((minor)<<16)|((revision)<<8))

/// Retrieves the major version from a packed system version.
#define GET_VERSION_MAJOR(version)    ((version) >>24)

/// Retrieves the minor version from a packed system version.
#define GET_VERSION_MINOR(version)    (((version)>>16)&0xFF)

/// Retrieves the revision version from a packed system version.
#define GET_VERSION_REVISION(version) (((version)>> 8)&0xFF)

/// Memory regions.
typedef enum
{
	MEMREGION_ALL = 0,         ///< All regions.
	MEMREGION_APPLICATION = 1, ///< APPLICATION memory.
	MEMREGION_SYSTEM = 2,      ///< SYSTEM memory.
	MEMREGION_BASE = 3,        ///< BASE memory.
} MemRegion;

/// Tick counter.
typedef struct
{
	u64 elapsed;   ///< Elapsed CPU ticks between measurements.
	u64 reference; ///< Point in time used as reference.
} TickCounter;

/// OS_VersionBin. Format of the system version: "<major>.<minor>.<build>-<nupver><region>"
typedef struct
{
	u8 build;
	u8 minor;
	u8 mainver;//"major" in CVER, NUP version in NVer.
	u8 reserved_x3;
	char region;//"ASCII character for the system version region"
	u8 reserved_x5[0x3];
} OS_VersionBin;

/**
 * @brief Converts an address from virtual (process) memory to physical memory.
 * @param vaddr Input virtual address.
 * @return The corresponding physical address.
 * It is sometimes required by services or when using the GPU command buffer.
 */
u32 osConvertVirtToPhys(const void* vaddr);

/**
 * @brief Converts 0x14* vmem to 0x30*.
 * @param vaddr Input virtual address.
 * @return The corresponding address in the 0x30* range, the input address if it's already within the new vmem, or 0 if it's outside of both ranges.
 */
void* osConvertOldLINEARMemToNew(const void* vaddr);

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
static inline u32 osGetFirmVersion(void)
{
	return (*(vu32*)0x1FF80060) & ~0xFF;
}

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
static inline u32 osGetKernelVersion(void)
{
	return (*(vu32*)0x1FF80000) & ~0xFF;
}

/**
 * @brief Gets the size of the specified memory region.
 * @param region Memory region to check.
 * @return The size of the memory region, in bytes.
 */
static inline u32 osGetMemRegionSize(MemRegion region)
{
	if(region == MEMREGION_ALL) {
		return osGetMemRegionSize(MEMREGION_APPLICATION) + osGetMemRegionSize(MEMREGION_SYSTEM) + osGetMemRegionSize(MEMREGION_BASE);
	} else {
		return *(vu32*) (0x1FF80040 + (region - 1) * 0x4);
	}
}

/**
 * @brief Gets the number of used bytes within the specified memory region.
 * @param region Memory region to check.
 * @return The number of used bytes of memory.
 */
s64 osGetMemRegionUsed(MemRegion region);

/**
 * @brief Gets the number of free bytes within the specified memory region.
 * @param region Memory region to check.
 * @return The number of free bytes of memory.
 */
static inline s64 osGetMemRegionFree(MemRegion region)
{
	return (s64) osGetMemRegionSize(region) - osGetMemRegionUsed(region);
}

/**
 * @brief Gets the current time.
 * @return The number of milliseconds since 1st Jan 1900 00:00.
 */
u64 osGetTime(void);

/**
 * @brief Starts a tick counter.
 * @param cnt The tick counter.
 */
static inline void osTickCounterStart(TickCounter* cnt)
{
	cnt->reference = svcGetSystemTick();
}

/**
 * @brief Updates the elapsed time in a tick counter.
 * @param cnt The tick counter.
 */
static inline void osTickCounterUpdate(TickCounter* cnt)
{
	u64 now = svcGetSystemTick();
	cnt->elapsed = now - cnt->reference;
	cnt->reference = now;
}

/**
 * @brief Reads the elapsed time in a tick counter.
 * @param cnt The tick counter.
 * @return The number of milliseconds elapsed.
 */
double osTickCounterRead(TickCounter* cnt);

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
static inline u8 osGetWifiStrength(void)
{
	return *(vu8*)0x1FF81066;
}

/**
 * @brief Gets the state of the 3D slider.
 * @return The state of the 3D slider (0.0~1.0)
 */
static inline float osGet3DSliderState(void)
{
	return *(volatile float*)0x1FF81080;
}

/**
 * @brief Configures the New 3DS speedup.
 * @param enable Specifies whether to enable or disable the speedup.
 */
void osSetSpeedupEnable(bool enable);

/**
 * @brief Gets the NAND system-version stored in NVer/CVer.
 * @param nver_versionbin Output OS_VersionBin structure for the data read from NVer.
 * @param cver_versionbin Output OS_VersionBin structure for the data read from CVer.
 * @return The result-code. This value can be positive if opening "romfs:/version.bin" fails with stdio, since errno would be returned in that case. In some cases the error can be special negative values as well.
 */
Result osGetSystemVersionData(OS_VersionBin *nver_versionbin, OS_VersionBin *cver_versionbin);

/**
 * @brief This is a wrapper for osGetSystemVersionData.
 * @param nver_versionbin Optional output OS_VersionBin structure for the data read from NVer, can be NULL.
 * @param cver_versionbin Optional output OS_VersionBin structure for the data read from CVer, can be NULL.
 * @param sysverstr Output string where the printed system-version will be written, in the same format displayed by the System Settings title.
 * @param sysverstr_maxsize Max size of the above string buffer, *including* NULL-terminator.
 * @return See osGetSystemVersionData.
 */
Result osGetSystemVersionDataString(OS_VersionBin *nver_versionbin, OS_VersionBin *cver_versionbin, char *sysverstr, u32 sysverstr_maxsize);
