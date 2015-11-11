///////////////////////////////////////
//           SDMC example            //
///////////////////////////////////////

//this example shows you how to load a binary image file from the SD card and display it on the lower screen
//for this to work you should copy test.bin to same folder as your .3dsx
//this file was generated with GIMP by saving a 240x320 image to raw RGB
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <3ds.h>
#include "costable.h"

//this will contain the data read from SDMC
u8* buffer;

//3DS has VFPs so we could just use cos
//but we're old school so LUT4life
s32 pcCos(u16 v)
{
	return costable[v&0x1FF];
}

void renderEffect()
{
	static int cnt;
	u8* bufAdr=gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);

	int i, j;
	for(i=1;i<400;i++)
	{
		for(j=1;j<240;j++)
		{
			u32 v=(j+i*240)*3;
			bufAdr[v]=(pcCos(i+cnt)+4096)/32;
			bufAdr[v+1]=(pcCos(j-256+cnt)+4096)/64;
			bufAdr[v+2]=(pcCos(i+128-cnt)+4096)/32;
		}
	}

	cnt++;
}

int main(int argc, char** argv)
{

	gfxInitDefault(); //makes displaying to screen easier

	FILE *file = fopen("test.bin","rb");
	if (file == NULL) goto exit;

	// seek to end of file
	fseek(file,0,SEEK_END);

	// file pointer tells us the size
	off_t size = ftell(file);

	// seek back to start
	fseek(file,0,SEEK_SET);

	//allocate a buffer
	buffer=malloc(size);
	if(!buffer)goto exit;

	//read contents !
	off_t bytesRead = fread(buffer,1,size,file);

	//close the file because we like being nice and tidy
	fclose(file);

	if(size!=bytesRead)goto exit;

	while(aptMainLoop())
	{
		//exit when user hits B
		hidScanInput();
		if(keysHeld()&KEY_B)break;

		//render rainbow
		renderEffect();

		//copy buffer to lower screen (don't have to do it every frame)
		memcpy(gfxGetFramebuffer(GFX_BOTTOM, GFX_BOTTOM, NULL, NULL), buffer, size);

		//wait & swap
		gfxSwapBuffersGpu();
		gspWaitForEvent(GSPGPU_EVENT_VBlank0, false);
	}

	//cleanup and return
	//returning from main() returns to hbmenu when run under ninjhax
	exit:

	//closing all services even more so
	gfxExit();
	return 0;
}
