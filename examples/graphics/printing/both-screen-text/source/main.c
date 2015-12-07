/*
	Both Screen Text example made by Aurelio Mannara for ctrulib
	This code was modified for the last time on: 12/12/2014 23:20 UTC+1

*/

#include <3ds.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	// Initialize services
	gfxInitDefault();

	//In this example we need one PrintConsole for each screen
	PrintConsole topScreen, bottomScreen;

	//Initialize console for both screen using the two different PrintConsole we have defined
	consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);

	//Before doing any text printing we should select the PrintConsole in which we are willing to write, otherwise the library will print on the last selected/initialized one
	//Let's start by printing something on the top screen
	consoleSelect(&topScreen);
	printf("This is the top screen. We have lot of space here!\n");

	//Now write something else on the bottom screen
	consoleSelect(&bottomScreen);
	printf("This is the bottom screen. We don't have much space...\nStill better than DS screens :D");

	consoleSelect(&topScreen);
	printf("\x1b[29;15HPress Start to exit.");

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
	
	return 0;
}
