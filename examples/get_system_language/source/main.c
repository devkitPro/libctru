#include <stdio.h>
#include <string.h>
#include <3ds.h>


int main(int argc, char** argv)
{
	// Initialize services
	gfxInit();
	initCfgu();


	u8 language = 0;

	// Init console for text output
	consoleInit(GFX_BOTTOM, NULL);

	// Read the language field from the config savegame.
	// See here for more block IDs:
	// http://3dbrew.org/wiki/Config_Savegame#Configuration_blocks
	CFGU_GetConfigInfoBlk2(1, 0xA0002, &language);


	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();

		// Print language code
		printf("\x1b[0;0HLanguage code: %d", (int)language);

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}

	// Exit services
	exitCfgu();
	gfxExit();
	return 0;
}
