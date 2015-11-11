#include <stdio.h>
#include <string.h>

#include <3ds.h>

#include "costable.h"

u8* inaddr;
u8* outaddr;

char logstring[256];

s32 pcCos(u16 v)
{
	return costable[v&0x1FF];
}

void printstring(char *str)//Placeholder until ctrulib itself has proper text drawing.
{
	strncat(logstring, str, sizeof(logstring)-1);
}

void draw_startup()
{
	Result ret;

	FILE *f = NULL;

	u8* bufAdr = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	u8* gfxtopadr = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);

	MVDSTD_Config config;

	char str[256];

	int i, j;
	u32 cnt=0;
	for(i=0;i<320;i++)
	{
		for(j=0;j<240;j++)
		{
			u32 v=(j+i*240)*3;
			bufAdr[v]=(pcCos(i+cnt)+4096)/32;
			bufAdr[v+1]=(pcCos(j-256+cnt)+4096)/64;
			bufAdr[v+2]=(pcCos(i+128-cnt)+4096)/32;
		}
	}

	f = fopen("sdmc:/mvd_indata.bin", "r");
	if(f)
	{
		fread(inaddr, 1, 0x46500, f);
		fclose(f);
	}
	else
	{
		memcpy(inaddr, bufAdr, 320*240*3);
	}

	memset(gfxtopadr, 0, 0x46500);
	GSPGPU_FlushDataCache(inaddr, 0x46500);

	printstring("mvd example\n");

	ret = mvdstdInit(MVDMODE_COLORFORMATCONV, MVD_INPUT_YUYV422, MVD_OUTPUT_RGB565, 0);
	memset(str, 0, 256);
	snprintf(str, sizeof(str)-1, "mvdstdInit(): 0x%08x\n", (unsigned int)ret);
	printstring(str);

	if(ret>=0)
	{
		mvdstdGenerateDefaultConfig(&config, 320, 240, 320, 240, (u32*)inaddr, (u32*)outaddr, (u32*)&outaddr[0x12c00]);

		ret = mvdstdProcessFrame(&config, NULL, 0, 0);
		memset(str, 0, 256);
		snprintf(str, sizeof(str)-1, "mvdstdProcessFrame(): 0x%08x\n", (unsigned int)ret);
		printstring(str);
	}

	svcSleepThread(1000000000);//Not sure how to determine when frame processing finishes.

	GSPGPU_InvalidateDataCache(outaddr, 0x100000);

	f = fopen("sdmc:/mvd_outdata.bin", "w");
	if(f)
	{
		fwrite(outaddr, 1, 0x100000, f);
		fclose(f);
	}

	f = fopen("sdmc:/mvd_log", "w");
	if(f)
	{
		fwrite(logstring, 1, strlen(logstring), f);
		fclose(f);
	}

	memcpy(gfxtopadr, outaddr, 0x46500);

	mvdstdExit();

	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();
}

int main()
{
	gfxInitDefault();
	//gfxSet3D(true); // uncomment if using stereoscopic 3D

	memset(logstring, 0, 256);

	inaddr = linearAlloc(0x100000);
	outaddr = linearAlloc(0x100000);

	if(inaddr && outaddr)
	{
		memset(inaddr, 0, 0x100000);
		memset(outaddr, 0, 0x100000);
		draw_startup();
	}

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		// Your code goes here

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
	}

	if(inaddr)linearFree(inaddr);
	if(outaddr)linearFree(outaddr);

	gfxExit();
	return 0;
}

