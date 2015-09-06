#include <stdio.h>
#include <string.h>
#include <3ds.h>



int main()
{
	gfxInitDefault(); // Init graphic stuff


	// We need these 2 buffers for APT_DoAppJump() later. They can be smaller too
	u8 buf0[0x300];
	u8 buf1[0x20];


	// Loop as long as the status is not exit
	while(aptMainLoop())
	{
		// Scan hid shared memory for input events
		hidScanInput();

		if(hidKeysDown() & KEY_A) // If the A button got pressed, start the app launch
		{
			// Clear both buffers
			memset(buf0, 0, 0x300);
			memset(buf1, 0, 0x20);

			// Open an APT session so we can talk to the APT service
			aptOpenSession();
			// Prepare for the app launch
			APT_PrepareToDoAppJump(0, 0x0004001000022400LL, 0); // *EUR* camera app title ID
			// Tell APT to trigger the app launch and set the status of this app to exit
			APT_DoAppJump(0x300 /* size of buf0 */, 0x20 /* size of buf1 */, buf0, buf1);
			// Close the APT session because we don't need APT anymore
			aptCloseSession();
		}

		// Flush + swap framebuffers and wait for VBlank. Not really needed in this example
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}


	gfxExit();

	return 0;
}
