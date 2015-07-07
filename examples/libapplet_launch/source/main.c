#include <3ds.h>
#include <stdio.h>

static bool allowed = false;

// If you define this function, you can monitor/debug APT events
void _aptDebug(int a, int b)
{
	if (allowed)
		printf("_aptDebug(%d,%x)\n", a, b);
}

int main()
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
	allowed = true;

	printf("Press B to launch applet\n");

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		gfxFlushBuffers();

		// Launch the extrapad library applet when button B is pressed.
		if (kDown & KEY_B)
		{
			Result rc = APT_LaunchLibraryApplet(APPID_EXTRAPAD, 0, NULL, 0);
			if (rc) printf("APT_LaunchLibraryApplet: %08lX\n", rc);
		}
	}

	// Exit services
	gfxExit();
	return 0;
}
