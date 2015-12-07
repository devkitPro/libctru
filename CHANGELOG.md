
##great-refactor

* New features

  - The DSP (audio) is now available through the use of the ndsp API : 3ds/ndsp/ndps.h
  - The documentation is now automaticly generated and available at http://smealum.github.io/ctrulib/
  - Added Inter Process Communication helpers : 3ds/ipc.h
  - Added svc/services Result helpers : 3ds/result.h
  - Light-weight synchronization primitives are now available in 3ds/synchronization.h
  - Code to manage the new3DS CPU speed : osSetSpeedupEnable in 3ds/os.h

* Major changes :

  - Most of the GPU_* helpers that are now deprecated as they were only masking register usage. Only register names and basic command buffers functions should be used. The intent is to have seperate libraries to choose how to implement and use their own command buffers.
  - Services are now reference-counted. This was made so that you could use libraries without worrying too much of what they init, or for API that requires multiple services (eg. ndsp needs cfg:u).
  - Some init/exit functions were renamed to match the xyzInit/xyzExit naming convention. See commit 2797540 for more details.
  - Some service prefixes were changed (eg. Y2R -> Y2RU)
  - Services handle parameters that were not needed have been removed (those required to pass NULL to use the ctrulib handle).
  - MICU service refactor, and updated example

* Other changes :

  - Commits and pull requests are now built on travis to check that the library compiles, and to generate the documentation.
  - Most services now have at least basic documentation
  - Added void to function with no paramaters
  - Added fragment lightning registers and enums
  - GX_Set* functions were renamed to GX_*
  - TitleList Renamed to AM_TitleEntry (3ds/services/am.h)
  -  Added the missing struct and functions for Y2R (3ds/services/y2r.h)

* Fixes :

  - a35abcb APT fix status event issue
  - 6bdfa1a aptInit Wait for APT_RUNNING
  - 0e7755a Fix GPUCMD_AddSingleParam() C++ compatibility issue 	

## version 0 to 0.6.0

No changelog available