#include <string.h>
#include <stdio.h>
#include <3ds.h>

#define NUMTHREADS 3
#define STACKSIZE (4 * 1024)

volatile bool runThreads = true;

void threadMain(void *arg)
{
	u64 sleepDuration = 1000000ULL * (u32)arg;
	int i = 0;
	while (runThreads)
	{
		printf("thread%d says %d\n", (int)arg, i++);
		svcSleepThread(sleepDuration);
	}
}

int main(int argc, char** argv)
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	Thread threads[NUMTHREADS];
	int i;
	s32 prio = 0;
	svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
	printf("Main thread prio: 0x%lx\n", prio);

	for (i = 0; i < NUMTHREADS; i ++)
	{
		// The priority of these child threads must be higher (aka the value is lower) than that
		// of the main thread, otherwise there is thread starvation due to stdio being locked.
		threads[i] = threadCreate(threadMain, (void*)((i+1)*250), STACKSIZE, prio-1, -2, false);
		printf("created thread %d: %p\n", i, threads[i]);
	}

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	// tell threads to exit & wait for them to exit
	runThreads = false;
	for (i = 0; i < NUMTHREADS; i ++)
	{
		threadJoin(threads[i], U64_MAX);
		threadFree(threads[i]);
	}

	gfxExit();
	return 0;
}
