/**
 * @file os.h
 * @brief OS related stuff.
 */
#pragma once
#include "svc.h"

#define SYSCLOCK_SOC       (16756991)
#define SYSCLOCK_ARM9      (SYSCLOCK_SOC * 8)
#define SYSCLOCK_ARM11     (SYSCLOCK_ARM9 * 2)
#define SYSCLOCK_ARM11_NEW (SYSCLOCK_ARM11 * 3)

#define CPU_TICKS_PER_MSEC (SYSCLOCK_ARM11 / 1000.0)
#define CPU_TICKS_PER_USEC (SYSCLOCK_ARM11 / 1000000.0)

/// Packs a system version from its components.
#define SYSTEM_VERSION(major, minor, revision) \
	(((major)<<24)|((minor)<<16)|((revision)<<8))

/// Retrieves the major version from a packed system version.
#define GET_VERSION_MAJOR(version)    ((version) >>24)

/// Retrieves the minor version from a packed system version.
#define GET_VERSION_MINOR(version)    (((version)>>16)&0xFF)

/// Retrieves the revision version from a packed system version.
#define GET_VERSION_REVISION(version) (((version)>> 8)&0xFF)

#define OS_HEAP_AREA_BEGIN 0x08000000 ///< Start of the heap area in the virtual address space
#define OS_HEAP_AREA_END   0x0E000000 ///< End of the heap area in the virtual address space

#define OS_MAP_AREA_BEGIN  0x10000000 ///< Start of the mappable area in the virtual address space
#define OS_MAP_AREA_END    0x14000000 ///< End of the mappable area in the virtual address space

#define OS_OLD_FCRAM_VADDR 0x14000000 ///< Old pre-8.x linear FCRAM mapping virtual address
#define OS_OLD_FCRAM_PADDR 0x20000000 ///< Old pre-8.x linear FCRAM mapping physical address
#define OS_OLD_FCRAM_SIZE  0x8000000  ///< Old pre-8.x linear FCRAM mapping size (128 MiB)

#define OS_QTMRAM_VADDR    0x1E800000 ///< New3DS QTM memory virtual address
#define OS_QTMRAM_PADDR    0x1F000000 ///< New3DS QTM memory physical address
#define OS_QTMRAM_SIZE     0x400000   ///< New3DS QTM memory size (4 MiB; last 128 KiB reserved by kernel)

#define OS_MMIO_VADDR      0x1EC00000 ///< Memory mapped IO range virtual address
#define OS_MMIO_PADDR      0x10100000 ///< Memory mapped IO range physical address
#define OS_MMIO_SIZE       0x400000   ///< Memory mapped IO range size (4 MiB)

#define OS_VRAM_VADDR      0x1F000000 ///< VRAM virtual address
#define OS_VRAM_PADDR      0x18000000 ///< VRAM physical address
#define OS_VRAM_SIZE       0x600000   ///< VRAM size (6 MiB)

#define OS_DSPRAM_VADDR    0x1FF00000 ///< DSP memory virtual address
#define OS_DSPRAM_PADDR    0x1FF00000 ///< DSP memory physical address
#define OS_DSPRAM_SIZE     0x80000    ///< DSP memory size (512 KiB)

#define OS_KERNELCFG_VADDR 0x1FF80000 ///< Kernel configuration page virtual address
#define OS_SHAREDCFG_VADDR 0x1FF81000 ///< Shared system configuration page virtual address

#define OS_FCRAM_VADDR     0x30000000 ///< Linear FCRAM mapping virtual address
#define OS_FCRAM_PADDR     0x20000000 ///< Linear FCRAM mapping physical address
#define OS_FCRAM_SIZE      0x10000000 ///< Linear FCRAM mapping size (256 MiB)

#define OS_KernelConfig ((osKernelConfig_s const*)OS_KERNELCFG_VADDR) ///< Pointer to the kernel configuration page (see \ref osKernelConfig_s)
#define OS_SharedConfig ((osSharedConfig_s*)OS_SHAREDCFG_VADDR)       ///< Pointer to the shared system configuration page (see \ref osSharedConfig_s)

/// Kernel configuration page (read-only).
typedef struct
{
	u32 kernel_ver;
	u32 update_flag;
	u64 ns_tid;
	u32 kernel_syscore_ver;
	u8  env_info;
	u8  unit_info;
	u8  boot_env;
	u8  unk_0x17;
	u32 kernel_ctrsdk_ver;
	u32 unk_0x1c;
	u32 firmlaunch_flags;
	u8  unk_0x24[0xc];
	u32 app_memtype;
	u8  unk_0x34[0xc];
	u32 memregion_sz[3];
	u8  unk_0x4c[0x14];
	u32 firm_ver;
	u32 firm_syscore_ver;
	u32 firm_ctrsdk_ver;
} osKernelConfig_s;

/// Time reference information struct (filled in by PTM).
typedef struct
{
	u64 value_ms;   ///< Milliseconds elapsed since January 1900 when this structure was last updated
	u64 value_tick; ///< System ticks elapsed since boot when this structure was last updated
	s64 sysclock_hz;///< System clock frequency in Hz adjusted using RTC measurements (usually around \ref SYSCLOCK_ARM11)
	s64 drift_ms;   ///< Measured time drift of the system clock (according to the RTC) in milliseconds since the last update
} osTimeRef_s;

/// Shared system configuration page structure (read-only or read-write depending on exheader).
typedef struct
{
	vu32 timeref_cnt;
	u8   running_hw;
	u8   mcu_hwinfo;
	u8   unk_0x06[0x1A];
	volatile osTimeRef_s timeref[2];
	u8   wifi_macaddr[6];
	vu8  wifi_strength;
	vu8  network_state;
	u8   unk_0x68[0x18];
	volatile float slider_3d;
	vu8  led_3d;
	vu8  led_battery;
	vu8  unk_flag;
	u8   unk_0x87;
	u8   unk_0x88[0x18];
	vu64 menu_tid;
	vu64 cur_menu_tid;
	u8   unk_0xB0[0x10];
	vu8  headset_connected;
} osSharedConfig_s;

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
const char* osStrError(Result error);

/**
 * @brief Gets the system's FIRM version.
 * @return The system's FIRM version.
 *
 * This can be used to compare system versions easily with @ref SYSTEM_VERSION.
 */
static inline u32 osGetFirmVersion(void)
{
	return OS_KernelConfig->firm_ver &~ 0xFF;
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
	return OS_KernelConfig->kernel_ver &~ 0xFF;
}

/// Gets the system's "core version" (2 on NATIVE_FIRM, 3 on SAFE_FIRM, etc.)
static inline u32 osGetSystemCoreVersion(void)
{
	return OS_KernelConfig->kernel_syscore_ver;
}

/// Gets the system's memory layout ID (0-5 on Old 3DS, 6-8 on New 3DS)
static inline u32 osGetApplicationMemType(void)
{
	return OS_KernelConfig->app_memtype;
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
		return OS_KernelConfig->memregion_sz[region-1];
	}
}

/**
 * @brief Gets the number of used bytes within the specified memory region.
 * @param region Memory region to check.
 * @return The number of used bytes of memory.
 */
static inline u32 osGetMemRegionUsed(MemRegion region)
{
	s64 mem_used;
	svcGetSystemInfo(&mem_used, 0, region);
	return mem_used;
}

/**
 * @brief Gets the number of free bytes within the specified memory region.
 * @param region Memory region to check.
 * @return The number of free bytes of memory.
 */
static inline u32 osGetMemRegionFree(MemRegion region)
{
	return osGetMemRegionSize(region) - osGetMemRegionUsed(region);
}

/**
 * @brief Reads the latest reference timepoint published by PTM.
 * @return Structure (see \ref osTimeRef_s).
 */
osTimeRef_s osGetTimeRef(void);

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
double osTickCounterRead(const TickCounter* cnt);

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
	return OS_SharedConfig->wifi_strength;
}

/**
 * @brief Gets the state of the 3D slider.
 * @return The state of the 3D slider (0.0~1.0)
 */
static inline float osGet3DSliderState(void)
{
	return OS_SharedConfig->slider_3d;
}

/**
 * @brief Checks whether a headset is connected.
 * @return true or false.
 */
static inline bool osIsHeadsetConnected(void)
{
	return OS_SharedConfig->headset_connected != 0;
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
