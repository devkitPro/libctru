#include <3ds/types.h>
#include <3ds/srv.h>
#include <3ds/gfx.h>
#include <3ds/sdmc.h>
#include <3ds/services/apt.h>
#include <3ds/services/fs.h>
#include <3ds/services/hid.h>

void __attribute__((weak)) __appInit() {
	// Initialize services
	srvInit();
	aptInit();
	hidInit();

	fsInit();
	sdmcInit();
}
