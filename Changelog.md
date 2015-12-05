# Changelog

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
