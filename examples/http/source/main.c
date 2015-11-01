#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <3ds.h>

Result http_download(httpcContext *context)//This error handling needs updated with proper text printing once ctrulib itself supports that.
{
	Result ret=0;
	u8* framebuf_top;
	u32 statuscode=0;
	u32 size=0, contentsize=0;
	u8 *buf;

	ret = httpcBeginRequest(context);
	if(ret!=0)return ret;

	ret = httpcGetResponseStatusCode(context, &statuscode, 0);
	if(ret!=0)return ret;

	if(statuscode!=200)return -2;

	ret=httpcGetDownloadSizeState(context, NULL, &contentsize);
	if(ret!=0)return ret;

	printf("size: %"PRId32"\n",contentsize);
	gfxFlushBuffers();

	buf = (u8*)malloc(contentsize);
	if(buf==NULL)return -1;
	memset(buf, 0, contentsize);


	ret = httpcDownloadData(context, buf, contentsize, NULL);
	if(ret!=0)
	{
		free(buf);
		return ret;
	}

	size = contentsize;
	if(size>(240*400*3*2))size = 240*400*3*2;

	framebuf_top = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	memcpy(framebuf_top, buf, size);

	gfxFlushBuffers();
	gfxSwapBuffers();

	framebuf_top = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	memcpy(framebuf_top, buf, size);

	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();

	free(buf);

	return 0;
}

int main()
{
	Result ret=0;
	httpcContext context;

	gfxInitDefault();
	httpcInit();

	consoleInit(GFX_BOTTOM,NULL);

	//Change this to your own URL.
	char *url = "http://devkitpro.org/misc/httpexample_rawimg.rgb";

	printf("Downloading %s\n",url);
	gfxFlushBuffers();

	ret = httpcOpenContext(&context, url, 1);
	printf("return from httpcOpenContext: %"PRId32"\n",ret);
	gfxFlushBuffers();

	if(ret==0)
	{
		ret=http_download(&context);
		printf("return from http_download: %"PRId32"\n",ret);
		gfxFlushBuffers();
		httpcCloseContext(&context);
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

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	// Exit services
	httpcExit();
	gfxExit();
	return 0;
}

