#include <string.h>
#include <malloc.h>
#include <inttypes.h>
#include <stdio.h>


#include <3ds.h>

Handle threadHandle, threadRequest;

#define STACKSIZE (4 * 1024)

volatile bool threadExit = false;

volatile int threadcount=0;

void threadMain(void *arg) {

	while(1) {
		svcWaitSynchronization(threadRequest, U64_MAX);
		svcClearEvent(threadRequest);		

		if(threadExit) svcExitThread();

		threadcount++;
	}
}

int main(int argc, char** argv) {


	gfxInitDefault();

	consoleInit(GFX_TOP, NULL);


	svcCreateEvent(&threadRequest,0);
	u32 *threadStack = memalign(32, STACKSIZE);
	Result ret = svcCreateThread(&threadHandle, threadMain, 0, &threadStack[STACKSIZE/4], 0x3f, 0);

	printf("thread create returned %x\n", ret);

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		printf("\x1b[5;0H");
		printf("thread counter = %d\n",threadcount);

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		if (kDown * KEY_A)
			svcSignalEvent(threadRequest);

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	// tell thread to exit
	threadExit = true;

	// signal the thread
	svcSignalEvent(threadRequest);

	// give it time to exit
	svcSleepThread(10000000ULL);

	// close handles and free allocated stack
	svcCloseHandle(threadRequest);
	svcCloseHandle(threadHandle);
	free(threadStack);


	gfxExit();
	return 0;
}
