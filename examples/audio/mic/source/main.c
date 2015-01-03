#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <3ds.h>

int main()
{
	u8 *framebuf;
	u32 *sharedmem = NULL, sharedmem_size = 0x30000;
	u8 *audiobuf;
	u32 audiobuf_size = 0x100000, audiobuf_pos = 0;
	u8 control=0x40;
	u32 audio_initialized = 0;

	srvInit();	
	aptInit();
	gfxInitDefault();
	hidInit(NULL);
	
	if(CSND_initialize(NULL)==0)audio_initialized = 1;

	sharedmem = (u32*)memalign(0x1000, sharedmem_size);
	audiobuf = linearAlloc(audiobuf_size);

	MIC_Initialize(sharedmem, sharedmem_size, control, 0, 3, 1, 1);//See mic.h.

	while(aptMainLoop())
	{
		hidScanInput();
		gspWaitForVBlank();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		if(audio_initialized)
		{
			framebuf = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);

			if(kDown & KEY_A)
			{
				audiobuf_pos = 0;

				CSND_setchannel_playbackstate(0x8, 0);//Stop audio playback.
				CSND_sharedmemtype0_cmdupdatestate(0);

				MIC_SetRecording(1);

				memset(framebuf, 0x20, 0x46500);
			}

			if((hidKeysHeld() & KEY_A) && audiobuf_pos < audiobuf_size)
			{
				audiobuf_pos+= MIC_ReadAudioData(&audiobuf[audiobuf_pos], audiobuf_size-audiobuf_pos, 1);
				if(audiobuf_pos > audiobuf_size)audiobuf_pos = audiobuf_size;

				memset(framebuf, 0x60, 0x46500);
			}

			if(hidKeysUp() & KEY_A)
			{
				MIC_SetRecording(0);
				GSPGPU_FlushDataCache(NULL, audiobuf, audiobuf_pos);
				CSND_playsound(0x8, CSND_LOOP_DISABLE, CSND_ENCODING_PCM16, 16000, (u32*)audiobuf, NULL, audiobuf_pos, 2, 0);

				memset(framebuf, 0xe0, 0x46500);

				gfxFlushBuffers();
				gfxSwapBuffers();

				framebuf = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
				memset(framebuf, 0xe0, 0x46500);
			}
		}

		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	MIC_Shutdown();

	if(audio_initialized)CSND_shutdown();

	free(sharedmem);
	linearFree(audiobuf);

	hidExit();
	gfxExit();
	aptExit();
	srvExit();
	return 0;
}

