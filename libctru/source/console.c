#include <stdio.h>
#include <string.h>
#include <sys/iosupport.h>
#include <3ds/gfx.h>
#include <3ds/console.h>

#include "default_font_bin.h"

//set up the palette for color printing
static u16 colorTable[] = {
	RGB565( 0, 0, 0),	// normal black
	RGB565(17, 0, 0),	// normal red
	RGB565( 0,15, 0),	// normal green
	RGB565(17,34, 0),	// normal yellow
	RGB565( 0, 0,17),	// normal blue
	RGB565(17, 0,17),	// normal magenta
	RGB565( 0,34,17),	// normal cyan
	RGB565(17,34,17),	// normal white
	RGB565( 0, 0, 0),	// bright black
	RGB565(25, 0, 0),	// bright red
	RGB565( 0,52, 0),	// bright green
	RGB565(25,52, 0),	// bright yellow
	RGB565( 4,18,31),	// bright blue
	RGB565(25, 0,25),	// bright magenta
	RGB565( 0,52,25),	// bright cyan
	RGB565(28,57,28)	// bright white
};

PrintConsole defaultConsole =
{
	//Font:
	{
		(u8*)default_font_bin, //font gfx
		0, //first ascii character in the set
		128 //number of characters in the font set
	},
	(u16*)NULL,
	0,0, 	//cursorX cursorY
	0,0,	 //prevcursorX prevcursorY
	40, 	//console width
	30, 	//console height
	0,  	//window x
	0,  	//window y
	40, 	//window width
	30, 	//window height
	3,		//tab size
	7,		// foreground color
	0,		// background color
	CONSOLE_COLOR_BRIGHT,	// flags
	0, 		//print callback
	false	//console initialized
};

PrintConsole currentCopy;

PrintConsole* currentConsole = &currentCopy;

PrintConsole* consoleGetDefault(void){return &defaultConsole;}

void consolePrintChar(char c);
void consoleDrawChar(int c);

//---------------------------------------------------------------------------------
static void consoleCls(char mode) {
//---------------------------------------------------------------------------------

	int i = 0;
	int colTemp,rowTemp;

	switch (mode)
	{
	case '[':
	case '0':
		{
			colTemp = currentConsole->cursorX ;
			rowTemp = currentConsole->cursorY ;

			while(i++ < ((currentConsole->windowHeight * currentConsole->windowWidth) - (rowTemp * currentConsole->consoleWidth + colTemp)))
				consolePrintChar(' ');

			currentConsole->cursorX  = colTemp;
			currentConsole->cursorY  = rowTemp;
			break;
		}
	case '1':
		{
			colTemp = currentConsole->cursorX ;
			rowTemp = currentConsole->cursorY ;

			currentConsole->cursorY  = 0;
			currentConsole->cursorX  = 0;

			while (i++ < (rowTemp * currentConsole->windowWidth + colTemp))
				consolePrintChar(' ');

			currentConsole->cursorX  = colTemp;
			currentConsole->cursorY  = rowTemp;
			break;
		}
	case '2':
		{
			currentConsole->cursorY  = 0;
			currentConsole->cursorX  = 0;

			while(i++ < currentConsole->windowHeight * currentConsole->windowWidth)
				consolePrintChar(' ');

			currentConsole->cursorY  = 0;
			currentConsole->cursorX  = 0;
			break;
		}
	}
}
//---------------------------------------------------------------------------------
static void consoleClearLine(char mode) {
//---------------------------------------------------------------------------------

	int i = 0;
	int colTemp;

	switch (mode)
	{
	case '[':
	case '0':
		{
			colTemp = currentConsole->cursorX ;

			while(i++ < (currentConsole->windowWidth - colTemp)) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		}
	case '1':
		{
			colTemp = currentConsole->cursorX ;

			currentConsole->cursorX  = 0;

			while(i++ < ((currentConsole->windowWidth - colTemp)-2)) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		}
	case '2':
		{
			colTemp = currentConsole->cursorX ;

			currentConsole->cursorX  = 0;

			while(i++ < currentConsole->windowWidth) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		}
	default:
		{
			colTemp = currentConsole->cursorX ;

			while(i++ < (currentConsole->windowWidth - colTemp)) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		}
	}
}


//---------------------------------------------------------------------------------
ssize_t con_write(struct _reent *r,int fd,const char *ptr, size_t len) {
//---------------------------------------------------------------------------------

	char chr;

	int i, count = 0;
	char *tmp = (char*)ptr;

	if(!tmp || len<=0) return -1;

	i = 0;

	while(i<len) {

		chr = *(tmp++);
		i++; count++;

		if ( chr == 0x1b && *tmp == '[' ) {
			bool escaping = true;
			char *escapeseq	= tmp;
			int escapelen = 0;

			do {
				chr = *(tmp++);
				i++; count++; escapelen++;
				int parameter, consumed, assigned;
				bool scanning;

				switch (chr) {
					//---------------------------------------
					// Cursor directional movement
					//---------------------------------------
					case 'A':
						assigned = sscanf(escapeseq,"[%dA", &parameter);
						if (assigned==0) parameter = 1;
						currentConsole->cursorY  =  (currentConsole->cursorY  - parameter) < 0 ? 0 : currentConsole->cursorY  - parameter;
						escaping = false;
						break;
					case 'B':
						sscanf(escapeseq,"[%dB", &parameter);
						currentConsole->cursorY  =  (currentConsole->cursorY  + parameter) > currentConsole->windowHeight - 1 ? currentConsole->windowHeight - 1 : currentConsole->cursorY  + parameter;
						escaping = false;
						break;
					case 'C':
						sscanf(escapeseq,"[%dC", &parameter);
						currentConsole->cursorX  =  (currentConsole->cursorX  + parameter) > currentConsole->windowWidth - 1 ? currentConsole->windowWidth - 1 : currentConsole->cursorX  + parameter;
						escaping = false;
						break;
					case 'D':
						sscanf(escapeseq,"[%dD", &parameter);
						currentConsole->cursorX  =  (currentConsole->cursorX  - parameter) < 0 ? 0 : currentConsole->cursorX  - parameter;
						escaping = false;
						break;
					//---------------------------------------
					// Cursor position movement
					//---------------------------------------
					case 'H':
					case 'f':
						sscanf(escapeseq,"[%d;%df", &currentConsole->cursorY , &currentConsole->cursorX );
						escaping = false;
						break;
					//---------------------------------------
					// Screen clear
					//---------------------------------------
					case 'J':
						consoleCls(escapeseq[escapelen-2]);
						escaping = false;
						break;
					//---------------------------------------
					// Line clear
					//---------------------------------------
					case 'K':
						consoleClearLine(escapeseq[escapelen-2]);
						escaping = false;
						break;
					//---------------------------------------
					// Save cursor position
					//---------------------------------------
					case 's':
						currentConsole->prevCursorX  = currentConsole->cursorX ;
						currentConsole->prevCursorY  = currentConsole->cursorY ;
						escaping = false;
						break;
					//---------------------------------------
					// Load cursor position
					//---------------------------------------
					case 'u':
						currentConsole->cursorX  = currentConsole->prevCursorX ;
						currentConsole->cursorY  = currentConsole->prevCursorY ;
						escaping = false;
						break;
					//---------------------------------------
					// Color scan codes
					//---------------------------------------
					case 'm':
						escapeseq++;
						scanning = true;

// do while doesn't work at -O2
//						do {
							sscanf(escapeseq,"%d;%n", &parameter, &consumed);
							escapeseq += consumed;

							if (parameter == 0 ) {
								currentConsole->flags |= CONSOLE_COLOR_BRIGHT;
								currentConsole->flags &= ~CONSOLE_COLOR_REVERSE;
								currentConsole->bg = 0;
								currentConsole->fg = 7;
							} else if (parameter == 7) { // reverse video
								currentConsole->flags |= CONSOLE_COLOR_REVERSE;
							} else if (parameter == 2) { // half bright
								currentConsole->flags &= ~CONSOLE_COLOR_BRIGHT;
							} else if (parameter >= 30 && parameter <= 37) { // writing color
								currentConsole->fg = parameter - 30;
							} else if (parameter >= 40 && parameter <= 47) { // screen color
								currentConsole->bg = parameter - 40;
							}
							if(escapeseq >= tmp) scanning = false;
//						} while(scanning);

						escaping = false;
						break;
				}
			} while (escaping);
			continue;
		}

		consolePrintChar(chr);
	}

	return count;
}

static const devoptab_t dotab_stdout = {
	"con",
	0,
	NULL,
	NULL,
	con_write,
	NULL,
	NULL,
	NULL
};

static const devoptab_t dotab_null = {
	"null",
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

//---------------------------------------------------------------------------------
PrintConsole* consoleInit(PrintConsole* console) {
//---------------------------------------------------------------------------------

	static bool firstConsoleInit = true;

	if(firstConsoleInit) {
		devoptab_list[STD_OUT] = &dotab_stdout;
		devoptab_list[STD_ERR] = &dotab_stdout;

		setvbuf(stdout, NULL , _IONBF, 0);
		setvbuf(stderr, NULL , _IONBF, 0);

		firstConsoleInit = false;
	}

	if(console) {
		currentConsole = console;
	} else {
		console = currentConsole;
	}

	*currentConsole = defaultConsole;

	console->consoleInitialised = 1;

	gfxSetScreenFormat(GFX_BOTTOM,GSP_RGB565_OES);
	gfxSetDoubleBuffering(GFX_BOTTOM,false);
	console->frameBuffer = (u16*)gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);


	consoleCls('2');

	return currentConsole;

}
//---------------------------------------------------------------------------------
PrintConsole *consoleSelect(PrintConsole* console){
//---------------------------------------------------------------------------------
	PrintConsole *tmp = currentConsole;
	currentConsole = console;
	return tmp;
}

//---------------------------------------------------------------------------------
void consoleSetFont(PrintConsole* console, ConsoleFont* font){
//---------------------------------------------------------------------------------

	if(!console) console = currentConsole;

	console->font = *font;

}

//---------------------------------------------------------------------------------
static void newRow() {
//---------------------------------------------------------------------------------


	currentConsole->cursorY ++;

	if(currentConsole->cursorY  >= currentConsole->windowHeight)  {
		currentConsole->cursorY --;
		u16 *dst = &currentConsole->frameBuffer[(currentConsole->windowX * 8 * 240) + (239 - (currentConsole->windowY * 8))];
		u16 *src = dst - 8;

		int i,j;

		for (i=0; i<currentConsole->windowWidth*8; i++) {
			u32 *from=(u32*)src;
			u32 *to = (u32*)dst;
			for (j=0; j<((currentConsole->windowHeight*8)-8)/2;j++) *(to--) = *(from--);

			dst += 240;
			src += 240;
		}

		consoleClearLine('2');
	}
}
//---------------------------------------------------------------------------------
void consoleDrawChar(int c) {
//---------------------------------------------------------------------------------
	c -= currentConsole->font.asciiOffset;
	if ( c < 0 || c > currentConsole->font.numChars ) return;

	u8 *fontdata = currentConsole->font.gfx + (8 * c);

	int writingColor = currentConsole->fg;
	int screenColor = currentConsole->bg;

	if (currentConsole->flags & CONSOLE_COLOR_BRIGHT) {
		writingColor |= 8;
		screenColor |=8;
	}

	if (currentConsole->flags & CONSOLE_COLOR_REVERSE) {
		int tmp = writingColor;
		writingColor = screenColor;
		screenColor = tmp;
	}

	u16 bg = colorTable[screenColor];
	u16 fg = colorTable[writingColor];

	u8 b1 = *(fontdata++);
	u8 b2 = *(fontdata++);
	u8 b3 = *(fontdata++);
	u8 b4 = *(fontdata++);
	u8 b5 = *(fontdata++);
	u8 b6 = *(fontdata++);
	u8 b7 = *(fontdata++);
	u8 b8 = *(fontdata++);

	u8 mask = 0x80;


	int i;

	int x = (currentConsole->cursorX + currentConsole->windowX) * 8;
	int y = ((currentConsole->cursorY + currentConsole->windowY) *8 );

	u16 *screen = &currentConsole->frameBuffer[(x * 240) + (239 - (y + 7))];

	for (i=0;i<8;i++) {
		if (b8 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b7 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b6 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b5 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b4 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b3 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b2 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b1 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		mask >>= 1;
		screen += 240 - 8;
	}

}

//---------------------------------------------------------------------------------
void consolePrintChar(char c) {
//---------------------------------------------------------------------------------
	if (c==0) return;

	if(currentConsole->PrintChar)
		if(currentConsole->PrintChar(currentConsole, c))
			return;

	if(currentConsole->cursorX  >= currentConsole->windowWidth) {
		currentConsole->cursorX  = 0;

		newRow();
	}

	switch(c) {
		/*
		The only special characters we will handle are tab (\t), carriage return (\r), line feed (\n)
		and backspace (\b).
		Carriage return & line feed will function the same: go to next line and put cursor at the beginning.
		For everything else, use VT sequences.

		Reason: VT sequences are more specific to the task of cursor placement.
		The special escape sequences \b \f & \v are archaic and non-portable.
		*/
		case 8:
			currentConsole->cursorX--;

			if(currentConsole->cursorX < 0) {
				if(currentConsole->cursorY > 0) {
					currentConsole->cursorX = currentConsole->windowX - 1;
					currentConsole->cursorY--;
				} else {
					currentConsole->cursorX = 0;
				}
			}

			consoleDrawChar(' ');
			break;

		case 9:
			currentConsole->cursorX  += currentConsole->tabSize - ((currentConsole->cursorX)%(currentConsole->tabSize));
			break;
		case 10:
			newRow();
		case 13:
			currentConsole->cursorX  = 0;
			break;
		default:
			consoleDrawChar(c);
			++currentConsole->cursorX ;
			break;
	}
}

//---------------------------------------------------------------------------------
void consoleClear(void) {
//---------------------------------------------------------------------------------
	iprintf("\x1b[2J");
}

//---------------------------------------------------------------------------------
void consoleSetWindow(PrintConsole* console, int x, int y, int width, int height){
//---------------------------------------------------------------------------------

	if(!console) console = currentConsole;

	console->windowWidth = width;
	console->windowHeight = height;
	console->windowX = x;
	console->windowY = y;

	console->cursorX = 0;
	console->cursorY = 0;

}


