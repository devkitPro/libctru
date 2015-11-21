#include <string.h>
#include <malloc.h>
#include <inttypes.h>
#include <stdio.h>


#include <3ds.h>

Thread threadHandle;
Handle threadRequest;

#define STACKSIZE (4 * 1024)

volatile bool runThread = true;

volatile int threadcount=0;

void threadMain(void *arg) {

	while(runThread) {
		svcWaitSynchronization(threadRequest, U64_MAX);
		svcClearEvent(threadRequest);

		threadcount++;
	}
}

int main(int argc, char** argv)
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	svcCreateEvent(&threadRequest,0);
	threadHandle = threadCreate(threadMain, 0, STACKSIZE, 0x3f, -2, true);

	printf("thread handle: %p\n", threadHandle);

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

		if (kDown & KEY_A)
			svcSignalEvent(threadRequest);

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	// tell thread to exit
	runThread = false;

	// signal the thread and wait for it to exit
	svcSignalEvent(threadRequest);
	threadJoin(threadHandle, U64_MAX);

	// close event handle
	svcCloseHandle(threadRequest);

	gfxExit();
	return 0;
}
