#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//might be missing some
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/linear.h>
#include <3ds/vram.h>
#include <3ds/os.h>
#include <3ds/gfx.h>
#include <3ds/console.h>
#include <3ds/util/utf.h>

#include <3ds/services/ac.h>
#include <3ds/services/am.h>
#include <3ds/services/apt.h>
#include <3ds/services/cam.h>
#include <3ds/services/cfgnor.h>
#include <3ds/services/cfgu.h>
#include <3ds/services/csnd.h>
#include <3ds/services/fs.h>
#include <3ds/services/gsp.h>
#include <3ds/services/hid.h>
#include <3ds/services/irrst.h>
#include <3ds/services/httpc.h>
#include <3ds/services/ir.h>
#include <3ds/services/ns.h>
#include <3ds/services/pm.h>
#include <3ds/services/ps.h>
#include <3ds/services/ptm.h>
#include <3ds/services/soc.h>
#include <3ds/services/mic.h>
#include <3ds/services/mvd.h>
#include <3ds/services/news.h>
#include <3ds/services/qtm.h>
#include <3ds/services/y2r.h>
#include <3ds/services/hb.h>

#include <3ds/gpu/gx.h>
#include <3ds/gpu/gpu.h>
#include <3ds/gpu/shbin.h>
#include <3ds/gpu/shaderProgram.h>

#include <3ds/sdmc.h>
#include <3ds/romfs.h>

#ifdef __cplusplus
}
#endif
/**
 * @example app_launch/source/main.c
 * @example audio/mic/source/main.c
 * @example get_system_language/source/main.c
 * @example graphics/bitmap/24bit-color/source/main.c
 * @example graphics/printing/hello-world/source/main.c
 * @example graphics/printing/both-screen-text/source/main.c
 * @example graphics/printing/colored-text/source/main.c
 * @example graphics/printing/multiple-windows-text/source/main.c

 * @example graphics/gpu/geoshader/source/main.c
   graphics/gpu/geoshader/source/gpu.h
   @include graphics/gpu/geoshader/source/gpu.h
   graphics/gpu/geoshader/source/gpu.c
   @include graphics/gpu/geoshader/source/gpu.c
   graphics/gpu/geoshader/source/3dmath.h
   @include graphics/gpu/geoshader/source/3dmath.h
   graphics/gpu/geoshader/source/3dmath.c
   @include graphics/gpu/geoshader/source/3dmath.c
   graphics/gpu/geoshader/source/vshader.pica
   @include graphics/gpu/geoshader/source/vshader.pica
   graphics/gpu/geoshader/source/gshader.pica
   @include graphics/gpu/geoshader/source/gshader.pica

   
 * @example graphics/gpu/simple_tri/source/main.c
   graphics/gpu/simple_tri/source/gpu.h
   @include graphics/gpu/simple_tri/source/gpu.h
   graphics/gpu/simple_tri/source/gpu.c
   @include graphics/gpu/simple_tri/source/gpu.c
   graphics/gpu/simple_tri/source/3dmath.h
   @include graphics/gpu/simple_tri/source/3dmath.h
   graphics/gpu/simple_tri/source/3dmath.c
   @include graphics/gpu/simple_tri/source/3dmath.c
   graphics/gpu/simple_tri/source/vshader.pica
   @include graphics/gpu/simple_tri/source/vshader.pica

   
 * @example graphics/gpu/textured_cube/source/main.c
   graphics/gpu/textured_cube/source/gpu.h
   @include graphics/gpu/textured_cube/source/gpu.h
   graphics/gpu/textured_cube/source/gpu.c
   @include graphics/gpu/textured_cube/source/gpu.c
   graphics/gpu/textured_cube/source/3dmath.h
   @include graphics/gpu/textured_cube/source/3dmath.h
   graphics/gpu/textured_cube/source/3dmath.c
   @include graphics/gpu/textured_cube/source/3dmath.c
   graphics/gpu/textured_cube/source/vshader.pica
   @include graphics/gpu/textured_cube/source/vshader.pica

 * @example http/source/main.c
 * @example input/read-controls/source/main.c
 * @example input/touch-screen/source/main.c
 * @example libapplet_launch/source/main.c
 * @example mvd/source/main.c
 * @example qtm/source/main.c
 * @example sdmc/source/main.c
 * @example threads/event/source/main.c
 * @example time/rtc/source/main.c
 */
 
