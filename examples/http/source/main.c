#include <stdlib.h>
#include <string.h>

#include <3ds.h>

Result http_download(httpcContext *context)//This error handling needs updated with proper text printing once ctrulib itself supports that.
{
	Result ret=0;
	u8* framebuf_top, *framebuf_bottom;
	u32 statuscode=0;
	u32 size=0, contentsize=0;
	u8 *buf;

	framebuf_bottom = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	memset(framebuf_bottom, 0x40, 240*320*3);
	gfxFlushBuffers();
	gfxSwapBuffers();

	framebuf_bottom = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	memset(framebuf_bottom, 0x40, 240*320*3);
	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();

	ret = httpcBeginRequest(context);
	if(ret!=0)return ret;

	ret = httpcGetResponseStatusCode(context, &statuscode, 0);
	if(ret!=0)return ret;

	if(statuscode!=200)return -2;

	ret=httpcGetDownloadSizeState(context, NULL, &contentsize);
	if(ret!=0)return ret;

	buf = (u8*)malloc(contentsize);
	if(buf==NULL)return -1;
	memset(buf, 0, contentsize);

	framebuf_bottom = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	memset(framebuf_bottom, 0xc0, 240*320*3);
	gfxFlushBuffers();
	gfxSwapBuffers();

	framebuf_bottom = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	memset(framebuf_bottom, 0xc0, 240*320*3);
	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();

	framebuf_top = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	framebuf_bottom = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);

	ret = httpcDownloadData(context, buf, contentsize, NULL);
	if(ret!=0)
	{
		free(buf);
		return ret;
	}

	size = contentsize;
	if(size>(240*400*3))size = 240*400*3;

	memset(framebuf_bottom, 0xff, 240*320*3);
	memcpy(framebuf_top, buf, size);

	gfxFlushBuffers();
	gfxSwapBuffers();

	framebuf_top = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	framebuf_bottom = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);

	memset(framebuf_bottom, 0xff, 240*320*3);
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
	//gfxSet3D(true); // uncomment if using stereoscopic 3D
	httpcInit();

	ret = httpcOpenContext(&context, "http://10.0.0.3/httpexample_rawimg.bin", 0);//Change this to your own URL.

	if(ret==0)
	{
		ret=http_download(&context);
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

