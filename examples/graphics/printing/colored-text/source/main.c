/*
	Colored Text example made by Aurelio Mannara for ctrulib
	This code was modified for the last time on: 12/12/2014 23:00 UTC+1

*/

#include <3ds.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	// Initialize services
	gfxInitDefault();

	//Initialize console on top screen. Using NULL as the second argument tells the console library to use the internal console structure as current one
	consoleInit(GFX_TOP, NULL);

	//Move the cursor to row 15 and column 19 and then prints "Hello World!" 
	//To move the cursor you have tu print "\x1b[r;cH", where r and c are respectively
	//the row and column where you want your cursor to move
	//The top screen has 30 rows and 50 columns
	//The bottom screen has 30 rows and 40 columns
	printf("\x1b[15;19HHello World!");

	//Move the cursor to the top left corner of the screen
	printf("\x1b[0;0H");

	//Print a REALLY crappy poeam with colored text
	//\x1b[cm set a SGR (Select Graphic Rendition) parameter, where c is the parameter that you want to set
	//Please refer to http://en.wikipedia.org/wiki/ANSI_escape_code#CSI_codes to see all the possible SGR parameters
	//As of now ctrulib support only these parameters:
	//Reset (0), Half bright colors (2), Reverse (7), Text color (30-37) and Background color (40-47)
	printf("Roses are \x1b[31mred\x1b[0m\n");
	printf("Violets are \x1b[34mblue\x1b[0m\n");
	printf("Piracy is bad\n");
	printf("While homebrews are good\n\n");

	//Black text on white background
	//In this example we set two parameter in a single escape sequence by separating them by a semicolon
	//\x1b[47;30m means that it will set a white background (47) and it will print white characters (30)
	//In this we also could have used the 
	printf("\x1b[47;30mBlack text on white background\x1b[0m");


	printf("\x1b[29;15HPress Start to exit.");
	// Main loop
	while (aptMainLoop())
	{
		//Scan all the inputs. This should be done once for each frame
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();

		if (kDown & KEY_START) break; // break in order to return to hbmenu

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();

		//Wait for VBlank
		gspWaitForVBlank();
	}

	// Exit services
	gfxExit();
	
	return 0;
}
