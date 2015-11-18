#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <3ds.h>

int main()
{
	gfxInitDefault();
	consoleInit(GFX_BOTTOM, NULL);

	bool initialized = true;

	u32 micbuf_size = 0x30000;
	u32 micbuf_pos = 0;
	u8* micbuf = memalign(0x1000, micbuf_size);

	printf("Initializing CSND...\n");
	if(R_FAILED(csndInit()))
	{
		initialized = false;
		printf("Could not initialize CSND.\n");
	} else printf("CSND initialized.\n");

	printf("Initializing MIC...\n");
	if(R_FAILED(micInit(micbuf, micbuf_size)))
	{
		initialized = false;
		printf("Could not initialize MIC.\n");
	} else printf("MIC initialized.\n");

	u32 micbuf_datasize = micGetSampleDataSize();

	u32 audiobuf_size = 0x100000;
	u32 audiobuf_pos = 0;
	u8* audiobuf = linearAlloc(audiobuf_size);

	if(initialized) printf("Hold A to record, release to play.\n");
	printf("Press START to exit.\n");

	while(aptMainLoop())
	{
		hidScanInput();
		gspWaitForVBlank();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		if(initialized)
		{
			if(kDown & KEY_A)
			{
				audiobuf_pos = 0;
				micbuf_pos = 0;

				printf("Stopping audio playback...\n");
				CSND_SetPlayState(0x8, 0);
				if(R_FAILED(CSND_UpdateInfo(0))) printf("Failed to stop audio playback.\n");

				printf("Starting sampling...\n");
				if(R_SUCCEEDED(MICU_StartSampling(MICU_ENCODING_PCM16_SIGNED, MICU_SAMPLE_RATE_16360, 0, micbuf_datasize, true))) printf("Now recording.\n");
				else printf("Failed to start sampling.\n");
			}

			if((hidKeysHeld() & KEY_A) && audiobuf_pos < audiobuf_size)
			{
				u32 micbuf_readpos = micbuf_pos;
				micbuf_pos = micGetLastSampleOffset();
				while(audiobuf_pos < audiobuf_size && micbuf_readpos != micbuf_pos)
				{
					audiobuf[audiobuf_pos] = micbuf[micbuf_readpos];
					audiobuf_pos++;
					micbuf_readpos = (micbuf_readpos + 1) % micbuf_datasize;
				}
			}

			if(hidKeysUp() & KEY_A)
			{
				printf("Stoping sampling...\n");
				if(R_FAILED(MICU_StopSampling())) printf("Failed to stop sampling.\n");

				printf("Starting audio playback...\n");
				if(R_SUCCEEDED(GSPGPU_FlushDataCache(audiobuf, audiobuf_pos)) && R_SUCCEEDED(csndPlaySound(0x8, SOUND_ONE_SHOT | SOUND_FORMAT_16BIT, 16360, 1.0, 0.0, (u32*)audiobuf, NULL, audiobuf_pos))) printf("Now playing.\n");
				else printf("Failed to start playback.\n");
			}
		}

		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	linearFree(audiobuf);

	micExit();
	free(micbuf);

	csndExit();
	gfxExit();
	return 0;
}

