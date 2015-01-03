#include <3ds.h>

int main()
{
	u32 val, i;

	// Initialize services
	srvInit();
	aptInit();
	hidInit(NULL);
	gfxInitDefault();
	//gfxSet3D(true); // uncomment if using stereoscopic 3D

	val = 0x22447899;

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		// Your code goes here

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		if (kDown & KEY_B)APT_LaunchLibraryApplet(0x408, 0, NULL, 0);//Launch the extrapad library applet when button B is pressed.

		// Example rendering code that displays a white pixel
		// Please note that the 3DS screens are sideways (thus 240x400 and 240x320)
		u32* fb = (u32*)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);

		for(i=0; i<(0x46500>>2); i++)fb[i] = val;

		val+= 0x44;

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	// Exit services
	gfxExit();
	hidExit();
	aptExit();
	srvExit();
	return 0;
}

