#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <3ds.h>

void printfile(const char* path)
{
	FILE* f = fopen(path, "r");
	if (f)
	{
		char mystring[100];
		while (fgets(mystring, sizeof(mystring), f))
		{
			int a = strlen(mystring);
			if (mystring[a-1] == '\n')
			{
				mystring[a-1] = 0;
				if (mystring[a-2] == '\r')
					mystring[a-2] = 0;
			}
			puts(mystring);
		}
		printf(">>EOF<<\n");
		fclose(f);
	}
}

int main()
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	Result rc = romfsInit();
	if (rc)
		printf("romfsInit: %08lX\n", rc);
	else
	{
		printf("romfs Init Successful!\n");
		printfile("folder/file.txt");
		// Test reading a file with non-ASCII characters in the name
		printfile("フォルダ/ファイル.txt");
	}

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
	}

	romfsExit();
	gfxExit();
	return 0;
}
