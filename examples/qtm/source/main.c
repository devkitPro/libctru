#include <string.h>
#include <stdio.h>
#include <3ds.h>

int main()
{
	u32 pos;
	u32 x, y;
	Result ret;
	bool qtm_usable;
	QTM_HeadTrackingInfo qtminfo;
	u32 colors[4] = {0x0000FF, 0x00FF00, 0xFF0000, 0xFFFFFF};

	gfxInitDefault();
	//gfxSet3D(true); // uncomment if using stereoscopic 3D

	qtmInit();

	consoleInit(GFX_BOTTOM, NULL);

	printf("qtm example\n");

	qtm_usable = qtmCheckInitialized();
	if(!qtm_usable)printf("QTM is not usable, therefore this example won't do anything with QTM.\n");

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		if(qtm_usable)
		{
			u8* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
			memset(fb, 0, 400*240*3);

			ret = QTM_GetHeadTrackingInfo(0, &qtminfo);
			if(ret==0)
			{
				consoleClear();

				for(pos=0; pos<5; pos++)
				{
					printf("flags[%x]=0x%x", (unsigned int)pos, qtminfo.flags[pos]);
					if(pos<4)printf(", ");
				}

				printf("\nfloatdata_x08: %f\n", qtminfo.floatdata_x08);

				printf("coords0: ");
				for(pos=0; pos<4; pos++)
				{
					printf("[%x].x=%f, y=%f", (unsigned int)pos, qtminfo.coords0[pos].x, qtminfo.coords0[pos].y);
					if(pos<3)printf(", ");
				}

				printf("\n");

				if(qtmCheckHeadFullyDetected(&qtminfo))
				{
					for(pos=0; pos<4; pos++)
					{
						ret = qtmConvertCoordToScreen(&qtminfo.coords0[pos], NULL, NULL, &x, &y);

						if(ret==0)memcpy(&fb[(x*240 + y) * 3], &colors[pos], 3);
					}
				}
			}
		}

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	// Exit services
	qtmExit();
	gfxExit();
	return 0;
}

