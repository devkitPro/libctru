///////////////////////////////////////
//           SDMC example            //
///////////////////////////////////////

//this example shows you how to load a binary image file from the SD card and display it on the lower screen
//for this to work you should copy test.bin to the root of your SD card
//this file was generated with GIMP by saving a 240x320 image to raw RGB
#include <string.h>

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
	//initialize the services we're going to be using
	srvInit(); //needed for everything
	aptInit(); //needed for everything
	hidInit(NULL); //needed for input
	gfxInit(); //makes displaying to screen easier
	fsInit(); //needed for filesystem stuff

	u64 size;
	u32 bytesRead;
	Handle fileHandle;
	//setup SDMC archive
	FS_archive sdmcArchive=(FS_archive){ARCH_SDMC, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	//create file path struct (note : FS_makePath actually only supports PATH_CHAR, it will change in the future)
	FS_path filePath=FS_makePath(PATH_CHAR, "/test.bin");

	//open file
	Result ret=FSUSER_OpenFileDirectly(NULL, &fileHandle, sdmcArchive, filePath, FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	//check for errors : exit if there is one
	if(ret)goto exit;

	//get file size
	ret=FSFILE_GetSize(fileHandle, &size);
	if(ret)goto exit;

	//allocate a buffer on linear heap (could just be a malloc fwiw)
	buffer=linearAlloc(size);
	if(!buffer)goto exit;

	//read contents !
	ret=FSFILE_Read(fileHandle, &bytesRead, 0x0, buffer, size);
	if(ret || size!=bytesRead)goto exit;

	//close the file because we like being nice and tidy
	ret=FSFILE_Close(fileHandle);
	if(ret)goto exit;
	
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
		gspWaitForEvent(GSPEVENT_VBlank0, false);
	}

	//cleanup and return
	//returning from main() returns to hbmenu when run under ninjhax
	exit:
	//closing all handles is super important
	svcCloseHandle(fileHandle);
	//closing all services even more so
	fsExit();
	gfxExit();
	hidExit();
	aptExit();
	srvExit();
	return 0;
}
