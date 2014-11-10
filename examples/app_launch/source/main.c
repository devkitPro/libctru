#include <stdio.h>
#include <string.h>
#include <3ds.h>



int main()
{
	srvInit();	
	aptInit();
	gfxInit();
	hidInit(NULL);


	u8 buf0[0x300];
	u8 buf1[0x20];


	while(aptMainLoop())
	{
		hidScanInput();
		svcSleepThread(100000000LL);

		if(hidKeysDown() & KEY_A)
		{
			memset(buf0, 0, 0x300);
			memset(buf1, 0, 0x20);

			aptOpenSession();
			APT_PrepareToDoAppJump(NULL, 0, 0x0004001000022400LL, 0); // *EUR* camera app
			APT_DoAppJump(NULL, 0x300, 0x20, buf0, buf1);
			aptCloseSession();
		}

		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}


	hidExit();
	gfxExit();
	aptExit();
	srvExit();

	return 0;
}
