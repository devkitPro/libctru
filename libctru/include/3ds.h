/**
 * @file 3ds.h
 * @brief Central 3DS header. Includes all others.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//might be missing some
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/ipc.h>
#include <3ds/svc.h>
#include <3ds/exheader.h>
#include <3ds/srv.h>
#include <3ds/errf.h>
#include <3ds/os.h>
#include <3ds/synchronization.h>
#include <3ds/thread.h>
#include <3ds/gfx.h>
#include <3ds/console.h>
#include <3ds/env.h>
#include <3ds/util/decompress.h>
#include <3ds/util/utf.h>

#include <3ds/allocator/linear.h>
#include <3ds/allocator/mappable.h>
#include <3ds/allocator/vram.h>

#include <3ds/services/ac.h>
#include <3ds/services/am.h>
#include <3ds/services/ampxi.h>
#include <3ds/services/apt.h>
#include <3ds/services/boss.h>
#include <3ds/services/cam.h>
#include <3ds/services/cfgnor.h>
#include <3ds/services/cfgu.h>
#include <3ds/services/csnd.h>
#include <3ds/services/dsp.h>
#include <3ds/services/fs.h>
#include <3ds/services/fspxi.h>
#include <3ds/services/fsreg.h>
#include <3ds/services/frd.h>
#include <3ds/services/gspgpu.h>
#include <3ds/services/gsplcd.h>
#include <3ds/services/hid.h>
#include <3ds/services/irrst.h>
#include <3ds/services/sslc.h>
#include <3ds/services/httpc.h>
#include <3ds/services/uds.h>
#include <3ds/services/ndm.h>
#include <3ds/services/nim.h>
#include <3ds/services/nwmext.h>
#include <3ds/services/ir.h>
#include <3ds/services/ns.h>
#include <3ds/services/pmapp.h>
#include <3ds/services/pmdbg.h>
#include <3ds/services/ps.h>
#include <3ds/services/ptmu.h>
#include <3ds/services/ptmsysm.h>
#include <3ds/services/pxidev.h>
#include <3ds/services/pxipm.h>
#include <3ds/services/soc.h>
#include <3ds/services/mic.h>
#include <3ds/services/mvd.h>
#include <3ds/services/nfc.h>
#include <3ds/services/news.h>
#include <3ds/services/qtm.h>
#include <3ds/services/srvpm.h>
#include <3ds/services/loader.h>
#include <3ds/services/y2r.h>
#include <3ds/services/mcuhwc.h>

#include <3ds/gpu/gx.h>
#include <3ds/gpu/gpu.h>
#include <3ds/gpu/shbin.h>
#include <3ds/gpu/shaderProgram.h>

#include <3ds/ndsp/ndsp.h>
#include <3ds/ndsp/channel.h>

#include <3ds/applets/swkbd.h>
#include <3ds/applets/error.h>

#include <3ds/applets/miiselector.h>

#include <3ds/archive.h>
#include <3ds/romfs.h>
#include <3ds/font.h>
#include <3ds/mii.h>

#include <3ds/gdbhio_dev.h>

#ifdef __cplusplus
}
#endif
/**
 * @example app_launch/source/main.c
 * @example audio/filters/source/main.c
 * @example audio/mic/source/main.c
 * @example audio/streaming/source/main.c
 * @example camera/image/source/main.c
 * @example camera/video/source/main.c
 * @example get_system_language/source/main.c
 * @example graphics/bitmap/24bit-color/source/main.c
 * @example graphics/gpu/both_screens/source/main.c
 * @example graphics/gpu/fragment_light/source/main.c
 * @example graphics/gpu/geoshader/source/main.c
 * @example graphics/gpu/gpusprites/source/main.c
 * @example graphics/gpu/immediate/source/main.c
 * @example graphics/gpu/lenny/source/main.c
 * @example graphics/gpu/loop_subdivision/source/main.c
 * @example graphics/gpu/mipmap_fog/source/main.c
 * @example graphics/gpu/particles/source/main.c
 * @example graphics/gpu/proctex/source/main.c
 * @example graphics/gpu/simple_tri/source/main.c
 * @example graphics/gpu/textured_cube/source/main.c
 * @example graphics/gpu/toon_shading/source/main.c
 * @example graphics/printing/both-screen-text/source/main.c
 * @example graphics/printing/colored-text/source/main.c
 * @example graphics/printing/hello-world/source/main.c
 * @example graphics/printing/multiple-windows-text/source/main.c
 * @example graphics/printing/system-font/source/main.c
 * @example input/read-controls/source/main.c
 * @example input/software-keyboard/source/main.c
 * @example input/touch-screen/source/main.c
 * @example libapplet_launch/source/main.c
 * @example mvd/source/main.c
 * @example network/boss/source/main.c
 * @example network/http/source/main.c
 * @example network/http_post/source/main.c
 * @example network/sockets/source/sockets.c
 * @example network/sslc/source/ssl.c
 * @example network/uds/source/uds.c
 * @example nfc/source/main.c
 * @example qtm/source/main.c
 * @example romfs/source/main.c
 * @example sdmc/source/main.c
 * @example threads/event/source/main.c
 * @example threads/thread-basic/source/main.c
 * @example time/rtc/source/main.c
 */
