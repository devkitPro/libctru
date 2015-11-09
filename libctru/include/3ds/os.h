/**
 * @file os.h
 *
 * OS related stuff.
 */
#pragma once

#define SYSTEM_VERSION(major, minor, revision) \
	(((major)<<24)|((minor)<<16)|((revision)<<8))

#define GET_VERSION_MAJOR(version)    ((version) >>24)
#define GET_VERSION_MINOR(version)    (((version)>>16)&0xFF)
#define GET_VERSION_REVISION(version) (((version)>> 8)&0xFF)

/*! OS_VersionBin. Format of the system version: "<major>.<minor>.<build>-<nupver><region>" */
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
 * Converts an address from virtual (process) memory to physical memory.
 * It is sometimes required by services or when using the GPU command buffer.
 */
u32 osConvertVirtToPhys(u32 vaddr);

/**
 * Converts 0x14* vmem to 0x30*.
 * @return The input address when it's already within the new vmem.
 * @return 0 when outside of either LINEAR mem areas.
 */
u32 osConvertOldLINEARMemToNew(u32 addr);

/**
 * @brief Basic information about a service error.
 * @return A string of the summary of an error.
 *
 * This can be used to get some details about an error returned by a service call.
 */
const char* osStrError(u32 error);

/**
 * @return the Firm version
 *
 * This can be used to compare system versions easily with @ref SYSTEM_VERSION.
 */
u32 osGetFirmVersion();

/**
 * @return the kernel version
 *
 * This can be used to compare system versions easily with @ref SYSTEM_VERSION.
 *
 * @code
 * if(osGetKernelVersion() > SYSTEM_VERSION(2,46,0)) printf("You are running 9.0 or higher\n");
 * @endcode
 */
u32 osGetKernelVersion();

/**
 * @return number of milliseconds since 1st Jan 1900 00:00.
 */
u64 osGetTime();

/**
 * @brief Returns the Wifi signal strength.
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
 *
 * @return the Wifi signal strength
 */
u8 osGetWifiStrength();

/**
 * @brief Configures the New 3DS speedup.
 * @param enable Specifies whether to enable or disable the speedup.
 */
void osSetSpeedupEnable(bool enable);

/**
 * @brief Gets the NAND system-version stored in NVer/CVer.
 * The romfs device must not be already initialized(via romfsInit*()) at the time this function is called, since this code uses the romfs device.
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

