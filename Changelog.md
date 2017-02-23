# Changelog

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
