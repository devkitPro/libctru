#include <stdio.h>
#include <string.h>
#include <3ds.h>


int main(int argc, char** argv)
{
	// Initialize services
	gfxInitDefault();
	cfguInit();


	u8 language = 0;
	Result res;

	// Init console for text output
	consoleInit(GFX_BOTTOM, NULL);

	// Read the language field from the config savegame.
	res = CFGU_GetSystemLanguage(&language);

	// Print return value and language code
	printf("       Result: 0x%x\n", (int)res);
	printf("Language code: %d", (int)language);


	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}

	// Exit services
	cfguExit();
	gfxExit();
	return 0;
}
