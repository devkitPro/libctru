/*
	Hello World example made by Aurelio Mannara for ctrulib
	This code was modified for the last time on: 12/13/2014 01:00 UTC+1

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
#include <string.h>

//This include a header containing definitions of our image
#include "drunkenlogo_rgb.h"

int main(int argc, char **argv)
{
	// Initialize services
	srvInit();
	aptInit();
	gfxInit();
	hidInit(NULL);

	//Initialize console on top screen. Using NULL as the second argument tells the console library to use the internal console structure as current one
	consoleInit(GFX_TOP, NULL);

	printf("Sorry for the recycled image from the Nintendo DS devkitPro's examples, but it's always good to bring back some memories, isn't it?");

	printf("\x1b[29;15HPress Start to exit.");

	//We don't need double buffering in this example. In this way we can draw our image only once on screen.
	gfxSetDoubleBuffering(GFX_BOTTOM, false);

	//Get the bottom screen's frame buffer
	u8* fb = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	
	//Copy our image in the bottom screen's frame buffer
	memcpy(fb, drunkenlogo_rgb, drunkenlogo_rgb_size);

	// Main loop
	while (aptMainLoop())
	{
		//Scan all the inputs. This should be done once for each frame
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();

		if (kDown & KEY_START) break; // break in order to return to hbmenu

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
