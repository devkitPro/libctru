/*
	Multiple Windows Text example made by Aurelio Mannara for ctrulib
	This code was modified for the last time on: 12/12/2014 23:50 UTC+1

*/

#include <3ds.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	// Initialize services
	gfxInitDefault();

	//In this example we need three PrintConsole, one for each window and one for the whole top screen
	PrintConsole leftWindow, rightWindow, topScreen;

	//Initialize console for each window
	consoleInit(GFX_TOP, &leftWindow);
	consoleInit(GFX_TOP, &rightWindow);
	consoleInit(GFX_TOP, &topScreen);

	//Now we specify the window position and dimension for each console window using consoleSetWindow(PrintConsole* console, int x, int y, int width, int height);
	//x, y, width and height are in terms of cells, not pixel, where each cell is composed by 8x8 pixels.
	consoleSetWindow(&leftWindow, 1, 1, 23, 28);
	consoleSetWindow(&rightWindow, 26, 1, 23, 28);

	//Before doing any text printing we should select the PrintConsole in which we are willing to write, otherwise the library will print on the last selected/initialized one
	//Let's start by printing something on the top screen
	consoleSelect(&leftWindow);
	printf("This text is in the left window!\n");
	printf("3DS rocks!!!\n");

	//Now write something else on the bottom screen
	consoleSelect(&rightWindow);
	printf("This text is in the right window!\n");
	printf("This thing works pretty well!\n");


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
