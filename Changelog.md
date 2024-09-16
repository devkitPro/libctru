# Changelog

## Version 2.4.0

- **Added full support of all QTM services**, with extensive documentation and technical details in `qtm.h` and `qtmc.h`** (breaking change); examples have been updated for this
- Added MCUHWC_SetInfoLedPattern
- Added ndspChnGetFormat
- Fixed PTMSYSM_CheckNew3DS
- Fixed NDMU_QueryStatus
- Fixed a few service commands improperly deserializing boolean output
- Fixed documentation of ACU_GetWifiStatus and ACU_SetAllowApType
- Fixed shaderInstanceInit to initialize all fields
- Further improvements to overall system stability and other minor adjustments have been made to enhance the user experience.

## Version 2.3.0

- Fix typo in docs by @DeltaF1 in https://github.com/devkitPro/libctru/pull/525
- Add GPU_DOT3_RGBA texture combiner function by @oreo639 in https://github.com/devkitPro/libctru/pull/528
- FSUSER_GetLegacyBannerData: Fix documentation typo by @Tekito-256 in https://github.com/devkitPro/libctru/pull/526
- Add CTR_ prefix to ALIGN,PACKED,DEPRECATED macros by @glebm in https://github.com/devkitPro/libctru/pull/532
- Buffer console control sequences by @piepie62 in https://github.com/devkitPro/libctru/pull/522
- Prevent CPU from postponing threadOnException memory writes
- Add SSID-related ac:i functions
  - ACI_LoadNetworkSetting, ACI_GetNetworkWirelessEssidSecuritySsid and acGetSessionHandle.
- Prevent double call of destructors on exit.
- Added experimental support for standard threading APIs (pthread, C threads, C++ std::thread)
- Further improvements to overall system stability and other minor adjustments have been made to enhance the user experience.

## New Contributors
- @DeltaF1 made their first contribution in https://github.com/devkitPro/libctru/pull/525
- @Tekito-256 made their first contribution in https://github.com/devkitPro/libctru/pull/526
- @glebm made their first contribution in https://github.com/devkitPro/libctru/pull/532

## Version 2.2.2

- archive_dev: Ensure path separator for local path
- adjust struct hostent for compatibilty

## Version 2.2.1

- add `_SOCKLEN_T_DECLARED`

## Version 2.2.0

- apt: add deliver arg support to chainloader

## Version 2.1.2

- Added cdc:CHK service wrappers
- Added support for `clock_gettime` (#495)
- svc: Fixed svcGetDmaState writing out-of-bounds data
- svc: Changed svcCreateCodeSet address parameters to pointer-sized integers, improve documentation
- fspxi: Fixed FSPXI_CreateFile and FSPXI_WriteFile (#496)
- ndsp: Added various ndspGet\* and ndspChnGet\* methods (#505, #506, #507)
- ir: Added IRU_GetSend/RecvFinishedEvent (#513)
- errf: Added ERRF_SetUserString and clarify documentation
- mcuhwc: Added mcuHwcGetSessionHandle
- apt: Fixed dirty homebrew chainload bug (used when Home Menu hasn't been started)
- Further improvements to overall system stability and other minor adjustments have been made to enhance the user experience.

## Version 2.1.1

- FPSCR is now initialized with a predictable value in all threads (including the main thread).
- Added gspGetSessionHandle and gspLcdGetSessionHandle.
- Fixed bugs related to uninitialized data in srv/errf service wrappers.
- Further improvements to overall system stability and other minor adjustments have been made to enhance the user experience.

## Version 2.1.0

**The #define for the 3DS platform has been changed to `__3DS__` (previously it was `_3DS`) - please update your Makefiles and your code**

### graphics

- Refactored VRAM allocators:
  - Added proper handling for VRAM banks (A and B, 3 MiB each)
  - Allocations no longer cross VRAM bank boundaries
  - Added vramAllocAt, vramMemAlignAt
- Add gspIsPresentPending.
- Add return value to gspPresentBuffer.
- libctru console now supports SGR 38 and 48 escape sequences (needed by fmtlib).
- Fixed GPU_TEXFACE enum.

### filesystem

- Changed rename to replace existing files, for better compatibility with POSIX (#483)
- Added SDMMC speed info types, and more clock rates (#480).

### miscellaneous

- Added support for 3dslink stdio redirection (#488).
- Added ptm:gets, ptm:sets and more ptm service commands.
- Added AM_ContentInfo, AM_ContentInfoFlags structs.
- Added AMAPP_GetDLCContentInfoCount, AMAPP_ListDLCContentInfos.
- Fixed bug in Huffman decoder.
- Further improvements to overall system stability and other minor adjustments have been made to enhance the user experience.

## Version 2.0.1

- Added CFG_SystemModel enum.
- Fixed bug in condvar code.
- Fixed bug in srvpm code.
- Fixed const correctness issues in gspgpu code.
- Further improvements to overall system stability and other minor adjustments have been made to enhance the user experience.

## Version 2.0.0

**This is a major release. Many essential components have been overhauled, although breaking changes are minimal and only affect rarely used functionality.**

### system

- Added support for userAppInit/userAppExit (backported from libnx).
- Changed default heap allocation logic to be more robust:
  - Resource limit SVCs are now used to detect available memory.
  - The heap size calculation algorithm was tweaked so that 32MB of linear heap are available for normal apps running under the default appmemtype layout on Old 3DS, as it was the case prior to libctru 1.8.0.
- SVC enhancements:
  - Added svcControlPerformanceCounter with corresponding enums.
  - Overhauled support for DMA related SVCs:
    - Added svcRestartDma.
    - Added enums and structures needed for DMA SVCs.
    - Added dmaDeviceConfigInitDefault, dmaConfigInitDefault.
  - Added svcArbitrateAddressNoTimeout (minor ABI optimization for the svcArbitrateAddress syscall when the timeout parameter is not used).
  - Changed process memory SVCs to use u32 parameters instead of void\* for foreign-process addresses.
- Major refactor of OS related functions:
  - Added defines for Arm11 userland memory region addresses/sizes.
  - Added struct definitions for the kernel/system shared config pages: osKernelConfig_s, osSharedConfig_s.
  - Added macros to access the kernel/system shared config pages: OS_KernelConfig, OS_SharedConfig.
  - Fixed return type of osGetMemRegionUsed, osGetMemRegionFree (s64 -> u32).
  - Refactored osConvertVirtToPhys and added support for the missing linearly mapped memory regions.
  - Rewritten RTC time reading support to improve accuracy and match official logic more closely.
    - Time drift correction is now implemented.
    - Added osGetTimeRef function for reading the current reference timepoint published by the PTM sysmodule.
  - Fixed osStrError to actually work properly.
  - Cleaned up osGetSystemVersionData implementation.
  - Other miscellaneous internal cleanup.

### synchronization

- Improved safety of usermode synchronization primitives when used in intercore contexts.
- Added CondVar synchronization primitive implementation.
- Added syncArbitrateAddress, syncArbitrateAddressWithTimeout.
  - These functions replace \_\_sync\_get\_arbiter (which has been removed).
- Added \_\_dmb (Data Memory Barrier) intrinsic.
- Added LightEvent_WaitTimeout.
- Added LightSemaphore_TryAcquire.
- Changed LightLock to support 0 as a valid initial (unlocked) state.
  - This effectively adds support for trivially initialized/zerofilled locks.
- Fixed bug in LightSemaphore_Acquire.

### graphics

- Major refactor of the GSP service wrapper. It should now be possible to use GSP directly without the gfx API, if the user so desires.
- Major internal refactor of the gfx wrapper API that increases maintainability.
- Added support for 800px wide mode with non-square pixels on the top screen - usable on every 3DS model except for Old 2DS (but *including* New 2DS XL).
- Added support for 800px wide mode to the libctru console.
- Transferred most of the GSP initialization duties to gspInit/gspExit (previously done by the gfx wrapper).
- Added defines for screen IDs and screen dimensions.
- Added gspPresentBuffer for pushing a framebuffer to the internal GSP swap queue (previously this was an internal function in the gfx wrapper).
- Added gspGetBytesPerPixel, gspHasGpuRight.
- Added gfxScreenSwapBuffers, with support for duplicating left->right eye content during stereoscopic 3D mode.
- Fixed LCD configuration mode when using framebuffers on VRAM with the gfx wrapper.
- Changed gfxExit to set LCD force-black status to true.
- Changed the gfx wrapper to always use gspPresentBuffer during swap. This means the immediate parameter of gfxConfigScreen no longer has any effect, and gfxSwapBuffers/gfxSwapBuffersGpu now do the same thing - that is, present the rendered content during the next VBlank.
- Renamed GSPGPU_FramebufferFormats enum to GSPGPU_FramebufferFormat.
- Deprecated gfxConfigScreen (use gfxScreenSwapBuffers instead).
- Removed gspInitEventHandler, gspExitEventHandler (now handled automatically inside gspInit/gspExit).
- Removed sharedGspCmdBuf param from gspSubmitGxCommand (now uses the proper GSP shared memory address automatically).
- Removed GSPGPU_REBASE_REG define (leftover from early 3DS homebrew).
- Removed numerous internal fields that were previously publicly accessible.

### audio

- DSP access rights are now managed using a hook mechanism, similar to the APT hook.
  - This fixes audio playback during libapplet invocations, as they no longer relinquish DSP rights.
- Changed NDSP to use the DSP hook instead of the APT hook.
- Added dspIsComponentLoaded, dspHook, dspUnhook.
- Fixed and improved robustness of wavebuf handling in NDSP code.
- Fixed and refactored NDSP sleep/wakeup code to improve accuracy compared to official logic.

### filesystem

- Reduced TLS footprint of the archive/romfs devices by sharing buffers.
- Reduced the maximum number of concurrently registered archive devices from 32 to 8 in order to save memory.
- Backported multi-mount romfs system from libnx, with additional optimizations (breaking API change).
- Added romfsMountFromTitle.
- Fixed stat for romfs device to return -1 for non-existent files/directories.

### applet

- Major internal refactor of the APT service wrapper that should improve accuracy compared to official logic.
- Fixed sleep handling when the app is inactive (i.e. app is suspended).
- Added support for screen capture during libapplet transitions.
- Added support for proper DSP access right management.
- aptLaunchLibraryApplet no longer calls aptMainLoop internally. Library applet calls no longer return a bool value, users are advised to check APT state manually afterwards.
- Added functions for checking APT state: aptIsActive, aptShouldClose, aptShouldJumpToHome, aptCheckHomePressRejected (replaces aptIsHomePressed).
- Added functions for handling incoming requests: aptHandleSleep, aptHandleJumpToHome.
- Added aptJumpToHomeMenu.
- Changed aptMainLoop to use the new wrapper functions (this means aptMainLoop can now be replaced by custom logic if desired).
- Changed APTHOOK_ONEXIT to be invoked during aptExit instead of during aptMainLoop.
- Added aptClearChainloader, aptSetChainloaderToSelf.

### miscellaneous

- Fix decompress out-of-bounds access.
- Added NDM_ prefix to NDM enum members in order to avoid name collisions.
- Corrected parameter type in several CFGU functions.
- Corrected return value of GSPLCD_GetBrightness.
- Removed obsolete support for ninjhax 1.x's fake hb:HB service.
- Further improvements to overall system stability and other minor adjustments have been made to enhance the user experience.

## Version 1.9.0

- hid: Added hidKeysDownRepeat, hidWaitForAnyEvent. Allow user override of irsst usage.
- Add ptm rtc time commands
- Add sleep state FSM handling service commands
- Revamp mappableAlloc
- Fixed stat on romfs for directories, implemented lstat via stat (FAT32 has no symlinks)
- Further improvements to overall system stability and other minor adjustments have been made to enhance the user experience.

## Version 1.8.0

- Added support for environments where the home menu is not launched (such as running under SAFE_FIRM)
- Added FSPXI service wrappers
- Changed heap allocation logic to be more flexible; this fixes support for certain system memory layouts
- Cleaned up and optimized light lock locking code
- Further improvements to overall system stability and other minor adjustments have been made to enhance the user experience.

## Version 1.7.0

- fix FIONBIO ioctl
- Fix CAMU_GetLatestVsyncTiming
- Add archive STDIO device driver
- Add AC commands that allow forcing a wifi connection
- Fix dspInit() error handling in ndspInit()
- Further improvements to overall system stability and other minor adjustments have been made to enhance the user experience.

## Version 1.6.0

- Major overhaul to font loading code in order to support loading external fonts.
- Major overhaul to PM service wrappers, now with separate support for pm:app and pm:dbg.
- Added Rosalina GDB host IO (gdbhio) support.
- Added support for the fs:REG service.
- Added support for the PxiPM service.
- Added PM launch flag definitions.
- Added SO_BROADCAST.
- Added new APT helper functions: aptIsHomeAllowed, aptSetHomeAllowed, aptIsHomePressed.
- Added support for the Mii selector libapplet, and other improvements to Mii support.
- Renamed IPC_Desc_CurProcessHandle to IPC_Desc_CurProcessId.
- Changed signature of LOADER_RegisterProgram to use FS_ProgramInfo structs.
- Fixed IPC bugs in the Loader service.
- Fixed svcCreateResourceLimit.
- Fixed corner case bug in GPUCMD_Add.
- Fixed bugs in select and herror.
- Further improvements to overall system stability and other minor adjustments have been made to enhance the user experience.

## Version 1.5.1

- Added support for the FRD service.
- Added gas-related GPU definitions.
- Implemented nanosleep.
- Further improvements to overall system stability and other minor adjustments have been made to enhance the user experience.

## Version 1.5.0

- Added new decompression API which supports LZSS/LZ10, LZ11, RLE & Huffman formats, and which can read compressed data from memory or from a file.
- Added srvSetBlockingPolicy, which controls whether srvGetServiceHandle blocks when the service isn't yet registered (or returns an error otherwise).
- Added ACU commands: ACU_GetStatus, ACU_GetSecurityMode, ACU_GetSSIDLength, ACU_GetSecurityMode, ACU_GetProxyEnable, ACU_GetProxyPort, ACU_GetProxyUserName, ACU_GetProxyPassword, ACU_GetLastErrorCode, ACU_GetLastDetailErrorCode.
- Added CFGI commands: CFGI_SecureInfoGetSerialNumber, CFGU_IsNFCSupported, CFGI_GetLocalFriendCodeSeed, CFGI_GetLocalFriendCodeSeedData, CFGI_GetSecureInfoData, CFGI_GetSecureInfoSignature.
- Added MCUHWC commands: MCUHWC_SetWifiLedState, MCUHWC_SetPowerLedState, MCUHWC_Get3dSliderLevel, MCUHWC_GetFwVerHigh, MCUHWC_GetFwVerLow.
- Added NS commands: NS_TerminateTitle, NS_TerminateProcessTID (with timeout), NS_RebootSystem.
- Added PM commands: PM_TerminateCurrentApplication, PM_TerminateProcess, PM_UnregisterProcess.
- Overhauled NDMU service support and added many commands that were previously missing.
- Fixed bugs in srv:pm implementation.
- Fixed calculation of vertical texture coordinates in fontCalcGlyphPos.
- Fixed shaderProgramSetGshInputPermutation.
- Fixed and added clock speed constants (affects system clock and NDSP).
- Further improvements to overall system stability and other minor adjustments have been made to enhance the user experience.

## Version 1.4.0

- Added LightSemaphore synchronization primitive.
- Added exheader definitions.
- Added support for the Loader service.
- Added support for the mcu::HWC service.
- Added support for the Mii selector applet.
- Added ResourceLimitType.
- Added AM commands: AM_DeleteAllTemporaryTitles, AM_DeleteAllExpiredTitles, AM_DeleteAllTwlTitles.
- Added CFGI commands: CFGI_RestoreLocalFriendCodeSeed, CFGI_RestoreSecureInfo, CFGI_DeleteConfigSavefile, CFGI_FormatConfig, CFGI_ClearParentalControls, CFGI_VerifySigLocalFriendCodeSeed, CFGI_VerifySigSecureInfo.
- Added GSPGPU commands: GSPGPU_SetLedForceOff.
- Added GSPLCD commands: GSPLCD_SetBrightness, GSPLCD_SetBrightnessRaw, GSPLCD_PowerOnAllBacklights, GSPLCD_PowerOffAllBacklights, GSPLCD_SetLedForceOff.
- Fixed srv:pm handling in pre-7.x system versions.
- Fixed GPU_LIGHTPERM macro definition.
- Removed the remaining deprecated GPUCMD commands.
- Further improvements to overall system stability and other minor adjustments have been made to enhance the user experience.

## Version 1.3.0

- Implement more svc calls
  - svcCreateResourceLimit
  - svcSetResourceLimitValues
  - svcSetProcessResourceLimits
  - svcCreateSession
  - svcCreateSessionToPort
  - svcSetGpuProt
  - svcSetWifiEnabled
- Additional functions
  - implement httpcAddPostDataBinary
  - implement PTMU_GetAdapterState
  - implement GSPLCD_GetBrightness
  - add threadDetach
- srv fixes
  - Fix srvPublishToSubscriber documentation
  - Fix handling of service/named port names of length 8
  - Fix srvRegisterPort
- debugging support
  - Add support for user-specified exception handlers
  - Rename debugDevice_3DMOO to debugDevice_SVC
  - created debug version of library
- Implement error applet
- GPU updates
  - Add GX command queue system for batching GX commands
  - Correct GPU_PROCTEX_LUTID definition
  - Add GPU_FOGMODE, GPU_GASMODE and GPU_GASLUTINPUT
- Other improvements and minor adjustments for overall system stability to enhance the user experience

## Version 1.2.1

  - Added NFC support
  - Update and renamed NFC_AmiiboConfig members
  - Added TickCounter for measuring performance
  - Added (linear/vram/mappable)GetSize for retrieving allocated buffer size
  - Added nim:s client implementation.
  - Correct bus clock used for time calulations
  - Default to file write without internal copy buffer
  - GPUCMD_Add: allow NULL for adding zerofilled parameter data
  - Clarify threadFree usage in documentation
  - Add GPU_TEXFACE enumeration

## Version 1.2.0

* New features:
  - Added support for the nfc:m/nfc:u service.
  - Added support for the ssl:C service.
  - Added support for the nwm::UDS service (Local wireless).
  - Added support for the boss:P/boss:U service (SpotPass).
  - Added support for using the err:f global port.
  - Added support for using and parsing the shared system font.
  - Added support for using the 3DS' built-in software keyboard applet.
  - Added support for using Light Events, a lightweight alternative to normal events that doesn't consume handles.
  - Added NDSP commands for setting up per-channel monopole & biquad filters.

* Major breaking changes:
  - Major revamp of the APT code; some old deprecated functions were completely removed, homebrew code abiding by recent standards shouldn't be affected though.
  - Major revamp of the HTTPC code, including both additions and bugfixes.
  - Major revamp of the AM code, with some enhancements.
  - Major revamp of FSUSER code, archive handle handling has been refactored. fs(End)UseSession replaced with fsExemptFromSession.
  - Major revamp of MVD code to support video processing.
  - Major revamp of the GPU shader code to fully support geometry shaders.
  - The old deprecated GPU wrapper commands have been removed. Use direct GPU register writes or a separate GPU wrapper library (such as citro3d) instead.
  - Major revamp and update of the SVC debugger API.
  - Heap size management has been simplified and made more flexible. All applications now default to 32MB of linear heap and all remaining APPLICATION memory is allocated as the application heap, regardless of entrypoint or application format.

* Miscellaneous additions:
  - Several enhancements to SOC:u support:
    - Added getaddrinfo, getnameinfo, gethostname, gai_strerror.
    - Added SOCU_ShutdownSockets, SOCU_CloseSockets, SOCU_GetIPInfo, SOCU_GetNetworkOpt, SOCU_AddGlobalSocket.
    - Added several missing flags.
  - Minor am:net corrections and additions.
  - Added sdmcWriteSafe to disable copying data to temporary RW buffers before calling FSFILE_Write.
  - Added sdmc_getmtime to retrieve the modification time for a file.
  - Added opendir/readdir/rewinddir/closedir/stat support to romfs.
  - Added support for multiple RomFS mounts.
  - Added macros for console color codes (ANSI escape sequences).
  - Added support for specifying a fallback RomFS path for when argv isn't available.
  - Added aptIsSleepAllowed/aptSetSleepAllowed.
  - Added ResetType enum for use with svcCreateEvent and svcCreateTimer.
  - Added psInitHandle and psGetSessionHandle.
  - Added AM commands: AM_ExportTwlBackup, AM_ImportTwlBackup, AM_ReadTwlBackupInfo, AM_DeleteAllDemoLaunchInfos, AM_FinishCiaInstallWithoutCommit, AM_CommitImportPrograms.
  - Added AMPXI commands: AMPXI_WriteTWLSavedata, AMPXI_InstallTitlesFinish.
  - Added APT commands: APT_ReceiveDeliverArg.
  - Added CFG commands: CFG_GetConfigInfoBlk4, CFG_GetConfigInfoBlk8, CFG_SetConfigInfoBlk4, CFG_SetConfigInfoBlk8, CFG_UpdateConfigNANDSavegame.
  - Added GSPLCD commands: GSPLCD_GetVendors
  - Added FSUSER commands: FSUSER_UpdateSha256Context.
  - Added NEWS commands: NEWS_GetTotalNotifications, NEWS_SetNotificationHeader, NEWS_GetNotificationHeader, NEWS_GetNotificationMessage, NEWS_GetNotificationImage, NEWS_SetNotificationMessage, NEWS_SetNotificationImage.
  - Added NS commands: NS_TerminateProcessTID, NS_LaunchFIRM, NS_LaunchApplicationFIRM.
  - Added PS commands: PS_SignRsaSha256, PS_VerifyRsaSha256.
  - Added PTMSYSM commands: PTMSYSM_CheckNew3DS, PTMSYSM_ShutdownAsync, PTMSYSM_RebootAsync.
  - Added PXIDEV commands: PXIDEV_SPIMultiWriteRead, PXIDEV_SPIWriteRead.
  - Added system calls: svcCreateCodeSet, svcCreateProcess, svcGetResourceLimit, svcGetResourceLimitValues, svcGetResourceLimitCurrentValues, svcSetProcessAffinityMask, svcSetProcessIdealProcessor, svcRun, svcBindInterrupt, svcUnbindInterrupt, svcGetHandleInfo, svcBreakRO, svcGetThreadList.
  - Added result module codes.
  - Added GPU A4 texture format enum.

* Miscellaneous changes and bug fixes:
  - Optimized sdmc's readdir to batch directory entries.
  - APT, GSPGPU and NDSP code was tuned to take advantage of Light Events.
  - Moved am:app init to a separate function.
  - Several issues concerning IPC static buffer saving/restoring were fixed.
  - Several issues concerning sdmc/RomFS devoptabs were fixed.
  - Several issues concerning SOC:u were fixed.
  - An issue concerning cam:u was fixed.
  - An issue concerning HID was fixed.
  - An issue concerning news:u was fixed.
  - The GPU ETC1 texture format enums were fixed.
  - usleep was fixed.
  - Fixed incorrect bool return values from services.
  - Fixed buffer overflow after gfxSetScreenFormat.
  - Fixed home menu display of suspended 2D applications.
  - Fixed buffer overrun in console code.
  - Fixed PS_EncryptDecryptAes and PS_EncryptSignDecryptVerifyAesCcm.
  - Fixed the implementation of svcGetProcessList.
  - Corrected svcKernelSetState function signature.

## Version 1.1.0

* Additions:
  - GSPGPU/GX code was revised and enhanced:
    - Screens can be buffer-swapped independently using the new gfxConfigScreen function.
    - Added gspSetEventCallback for running event code directly on the GSP thread.
    - Added gspWaitForAnyEvent for waiting for any GSP event.
    - Added gfxIs3D for retrieving 3D-enable status.
  - Added AM_InstallFirm.
  - Added __sync_get_arbiter.
  - Added support for usleep.

* Changes:
  - NDSP thread priority has been increased, therefore mitigating potential sound issues due to high CPU usage on the main thread.
  - RomFS initialization no longer makes romfs:/ the default device.

* Bug fixes:
  - Fixed the timeout parameter in svcArbitrateAddress.
  - Fixed svcSetTimer.

## Version 1.0.0

* New features:
  - libctru documentation is now available at http://smealum.github.io/ctrulib/
  - Added the NDSP API, which allows the use of the DSP (audio).
  - Added Inter Process Communication helpers.
  - Added Result code helpers.
  - Added support for lightweight synchronization primitives.
  - Added support for making the C/C++ standard libraries thread safe.
  - Added support for thread-local objects, with the use of standard C and C++ constructs (or GCC extensions).
  - Added a new threading API that properly manages internal state. Direct usage of svcCreateThread is deprecated.
  - Added a mappable address space allocator. Services which need to map shared memory blocks now use this allocator.
  - Added support for embedded RomFS, embedded SMDH and GPU shader building in the template Makefiles.

* Changes and additions to the GPU code:
  - Stateless wrapper functions (GPU_*) that merely masked GPU register usage were deprecated, in favour of external GPU wrapper libraries such as citro3d. A future release of libctru may remove them.
  - The API set has therefore been simplified down to command list management.
  - Synchronized register names with the 3dbrew Wiki.
  - Added fragment lighting registers and enums.
  - Added procedural texture registers and enums.
  - Added shaderProgramSetGshInputPermutation, for configuring the wiring between the vertex shader and the geometry shader.
  - Added shaderProgramSetGshMode, for configuring the geometry shader operation mode.
  - Added shaderProgramConfigure, intended to be used by GPU wrapper libraries.
  - SHBIN/shaderProgram code now correctly computes and sets the values of the GPUREG_SH_OUTATTR_MODE/CLOCK registers.
  - GX function naming has been improved, and the initial GX command buffer parameter has been removed.

* Major changes and miscellaneous additions:
  - Sweeping changes to make function/structure/enum naming more consistent across the whole library. This affects a lot of code.
  - Compiler/linker flags have been tweaked to increase performance and reduce code size, through the garbage collection of unused functions.
  - Service initialization is now reference counted in order to properly manage dependencies.
  - Initial service handle parameters have been removed, since they were nearly always set to NULL.
  - Completed coverage of srv and FSUSER service calls.
  - Added fsUseSession and fsEndUseSession for overriding the FSUSER session used in commands in the current thread.
  - Added osGet3DSliderState, osSetSpeedupEnable, osGetSystemVersionData and osGetSystemVersionDataString.
  - Refactored the MICU service.
  - NCCH versions of applications now detect the maximum amount of available memory on startup.

* Miscellaneous changes and bug fixes:
  - Commits and pull requests are now built on travis to check that the library compiles, and to generate the documentation.
  - General changes and improvements to overall code quality.
  - Added the missing struct and functions for Y2R.
  - Added srvGetServiceHandleDirect for bypassing the handle override mechanism.
  - Usage of the CSND service in new applications is not recommended, however it is not deprecated. The usage of NDSP instead is advised.
  - Usage of the HB service in new applications is not recommended due to its necessary removal in hax 2.x, however it is not deprecated.
  - Several bugs affecting APT were fixed.
  - Several bugs affecting C++ were fixed.

## Version 0 through 0.6.0

No changelog available.
