/*
	RTC example made by Aurelio Mannara for ctrulib
	This code was modified for the last time on: 12/13/2014 2:45 UTC+1

	This wouldn't be possible without the amazing work done by:
	-Smealum
	-fincs
	-WinterMute
	-yellows8
	-plutoo
	-mtheall
	-Many others who worked on 3DS and I'm surely forgetting about
*/

#include <3ds.h>
#include <stdio.h>

#define SECONDS_IN_DAY 86400
#define SECONDS_IN_HOUR 3600
#define SECONDS_IN_MINUTE 60

int main(int argc, char **argv)
{
	// Initialize services
	srvInit();
	aptInit();
	gfxInit();
	hidInit(NULL);

	//Initialize console on top screen. Using NULL as the second argument tells the console library to use the internal console structure as current one
	consoleInit(GFX_TOP, NULL);

	printf("\x1b[29;15HPress Start to exit.");

	// Main loop
	while (aptMainLoop())
	{
		//Scan all the inputs. This should be done once for each frame
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();

		if (kDown & KEY_START) break; // break in order to return to hbmenu

		//Print current time
		u64 timeInSeconds = osGetTime() / 1000;
		u64 dayTime = timeInSeconds % SECONDS_IN_DAY;
		u8 hour = dayTime / SECONDS_IN_HOUR;
		u8 min = (dayTime % SECONDS_IN_HOUR) / SECONDS_IN_MINUTE;
		u8 seconds = dayTime % SECONDS_IN_MINUTE;

		printf("\x1b[0;0H%02d:%02d:%02d", hour, min, seconds);

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();

		//Wait for VBlank
		gspWaitForVBlank();
	}

	// Exit services
	gfxExit();
	hidExit();
	aptExit();
	srvExit();
	return 0;
}
