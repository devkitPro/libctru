#include <stdio.h>
#include <string.h>
#include <sys/iosupport.h>
#include <3ds/gfx.h>
#include <3ds/console.h>
#include <3ds/svc.h>

#include "default_font_bin.h"

//set up the palette for color printing
static u16 colorTable[] = {
	RGB8_to_565(  0,  0,  0),	// black
	RGB8_to_565(128,  0,  0),	// red
	RGB8_to_565(  0,128,  0),	// green
	RGB8_to_565(128,128,  0),	// yellow
	RGB8_to_565(  0,  0,128),	// blue
	RGB8_to_565(128,  0,128),	// magenta
	RGB8_to_565(  0,128,128),	// cyan
	RGB8_to_565(192,192,192),	// white

	RGB8_to_565(128,128,128),	// bright black
	RGB8_to_565(255,  0,  0),	// bright red
	RGB8_to_565(  0,255,  0),	// bright green
	RGB8_to_565(255,255,  0),	// bright yellow
	RGB8_to_565(  0,  0,255),	// bright blue
	RGB8_to_565(255,  0,255),	// bright magenta
	RGB8_to_565(  0,255,255),	// bright cyan
	RGB8_to_565(255,255,255),	// bright white

	RGB8_to_565(  0,  0,  0),	// faint black
	RGB8_to_565( 64,  0,  0),	// faint red
	RGB8_to_565(  0, 64,  0),	// faint green
	RGB8_to_565( 64, 64,  0),	// faint yellow
	RGB8_to_565(  0,  0, 64),	// faint blue
	RGB8_to_565( 64,  0, 64),	// faint magenta
	RGB8_to_565(  0, 64, 64),	// faint cyan
	RGB8_to_565( 96, 96, 96),	// faint white
};

PrintConsole defaultConsole =
{
	//Font:
	{
		(u8*)default_font_bin, //font gfx
		0, //first ascii character in the set
		256 //number of characters in the font set
	},
	(u16*)NULL,
	0,0,	//cursorX cursorY
	0,0,	//prevcursorX prevcursorY
	40,		//console width
	30,		//console height
	0,		//window x
	0,		//window y
	40,		//window width
	30,		//window height
	3,		//tab size
	7,		// foreground color
	0,		// background color
	0,		// flags
	0,		//print callback
	false	//console initialized
};

PrintConsole currentCopy;

PrintConsole* currentConsole = &currentCopy;

PrintConsole* consoleGetDefault(void){return &defaultConsole;}

void consolePrintChar(int c);
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
	gfxFlushBuffers();
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
	}
	gfxFlushBuffers();
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
			char *escapeseq	= tmp++;
			int escapelen = 1;
			i++; count++;

			do {
				chr = *(tmp++);
				i++; count++; escapelen++;
				int parameter, assigned, consumed;

				// make sure parameters are positive values and delimited by semicolon
				if((chr >= '0' && chr <= '9') || chr == ';')
					continue;

				switch (chr) {
					//---------------------------------------
					// Cursor directional movement
					//---------------------------------------
					case 'A':
						consumed = 0;
						assigned = sscanf(escapeseq,"[%dA%n", &parameter, &consumed);
						if (assigned==0) parameter = 1;
						if (consumed)
							currentConsole->cursorY  =  (currentConsole->cursorY  - parameter) < 0 ? 0 : currentConsole->cursorY  - parameter;
						escaping = false;
						break;
					case 'B':
						consumed = 0;
						assigned = sscanf(escapeseq,"[%dB%n", &parameter, &consumed);
						if (assigned==0) parameter = 1;
						if (consumed)
							currentConsole->cursorY  =  (currentConsole->cursorY  + parameter) > currentConsole->windowHeight - 1 ? currentConsole->windowHeight - 1 : currentConsole->cursorY  + parameter;
						escaping = false;
						break;
					case 'C':
						consumed = 0;
						assigned = sscanf(escapeseq,"[%dC%n", &parameter, &consumed);
						if (assigned==0) parameter = 1;
						if (consumed)
							currentConsole->cursorX  =  (currentConsole->cursorX  + parameter) > currentConsole->windowWidth - 1 ? currentConsole->windowWidth - 1 : currentConsole->cursorX  + parameter;
						escaping = false;
						break;
					case 'D':
						consumed = 0;
						assigned = sscanf(escapeseq,"[%dD%n", &parameter, &consumed);
						if (assigned==0) parameter = 1;
						if (consumed)
							currentConsole->cursorX  =  (currentConsole->cursorX  - parameter) < 0 ? 0 : currentConsole->cursorX  - parameter;
						escaping = false;
						break;
					//---------------------------------------
					// Cursor position movement
					//---------------------------------------
					case 'H':
					case 'f':
					{
						int  x, y;
						char c;
						if(sscanf(escapeseq,"[%d;%d%c", &y, &x, &c) == 3 && (c == 'f' || c == 'H')) {
							currentConsole->cursorX = x;
							currentConsole->cursorY = y;
							escaping = false;
							break;
						}

						x = y = 1;
						if(sscanf(escapeseq,"[%d;%c", &y, &c) == 2 && (c == 'f' || c == 'H')) {
							currentConsole->cursorX = x;
							currentConsole->cursorY = y;
							escaping = false;
							break;
						}

						x = y = 1;
						if(sscanf(escapeseq,"[;%d%c", &x, &c) == 2 && (c == 'f' || c == 'H')) {
							currentConsole->cursorX = x;
							currentConsole->cursorY = y;
							escaping = false;
							break;
						}

						x = y = 1;
						if(sscanf(escapeseq,"[;%c", &c) == 1 && (c == 'f' || c == 'H')) {
							currentConsole->cursorX = x;
							currentConsole->cursorY = y;
							escaping = false;
							break;
						}

						// invalid format
						escaping = false;
						break;
					}
					//---------------------------------------
					// Screen clear
					//---------------------------------------
					case 'J':
						if(escapelen <= 3)
							consoleCls(escapeseq[escapelen-2]);
						escaping = false;
						break;
					//---------------------------------------
					// Line clear
					//---------------------------------------
					case 'K':
						if(escapelen <= 3)
							consoleClearLine(escapeseq[escapelen-2]);
						escaping = false;
						break;
					//---------------------------------------
					// Save cursor position
					//---------------------------------------
					case 's':
						if(escapelen == 2) {
							currentConsole->prevCursorX  = currentConsole->cursorX ;
							currentConsole->prevCursorY  = currentConsole->cursorY ;
						}
						escaping = false;
						break;
					//---------------------------------------
					// Load cursor position
					//---------------------------------------
					case 'u':
						if(escapelen == 2) {
							currentConsole->cursorX  = currentConsole->prevCursorX ;
							currentConsole->cursorY  = currentConsole->prevCursorY ;
						}
						escaping = false;
						break;
					//---------------------------------------
					// Color scan codes
					//---------------------------------------
					case 'm':
						escapeseq++;
						escapelen--;

						do {
							parameter = 0;
							if (escapelen == 1) {
								consumed = 1;
							} else if (strchr(escapeseq,';')) {
								sscanf(escapeseq,"%d;%n", &parameter, &consumed);
							} else {
								sscanf(escapeseq,"%dm%n", &parameter, &consumed);
							}

							escapeseq += consumed;
							escapelen -= consumed;

							switch(parameter) {
							case 0: // reset
								currentConsole->flags = 0;
								currentConsole->bg    = 0;
								currentConsole->fg    = 7;
								break;

							case 1: // bold
								currentConsole->flags &= ~CONSOLE_COLOR_FAINT;
								currentConsole->flags |= CONSOLE_COLOR_BOLD;
								break;

							case 2: // faint
								currentConsole->flags &= ~CONSOLE_COLOR_BOLD;
								currentConsole->flags |= CONSOLE_COLOR_FAINT;
								break;

							case 3: // italic
								currentConsole->flags |= CONSOLE_ITALIC;
								break;

							case 4: // underline
								currentConsole->flags |= CONSOLE_UNDERLINE;
								break;

							case 5: // blink slow
								currentConsole->flags &= ~CONSOLE_BLINK_FAST;
								currentConsole->flags |= CONSOLE_BLINK_SLOW;
								break;

							case 6: // blink fast
								currentConsole->flags &= ~CONSOLE_BLINK_SLOW;
								currentConsole->flags |= CONSOLE_BLINK_FAST;
								break;

							case 7: // reverse video
								currentConsole->flags |= CONSOLE_COLOR_REVERSE;
								break;

							case 8: // conceal
								currentConsole->flags |= CONSOLE_CONCEAL;
								break;

							case 9: // crossed-out
								currentConsole->flags |= CONSOLE_CROSSED_OUT;
								break;

							case 21: // bold off
								currentConsole->flags &= ~CONSOLE_COLOR_BOLD;
								break;

							case 22: // normal color
								currentConsole->flags &= ~CONSOLE_COLOR_BOLD;
								currentConsole->flags &= ~CONSOLE_COLOR_FAINT;
								break;

							case 23: // italic off
								currentConsole->flags &= ~CONSOLE_ITALIC;
								break;

							case 24: // underline off
								currentConsole->flags &= ~CONSOLE_UNDERLINE;
								break;

							case 25: // blink off
								currentConsole->flags &= ~CONSOLE_BLINK_SLOW;
								currentConsole->flags &= ~CONSOLE_BLINK_FAST;
								break;

							case 27: // reverse off
								currentConsole->flags &= ~CONSOLE_COLOR_REVERSE;
								break;

							case 29: // crossed-out off
								currentConsole->flags &= ~CONSOLE_CROSSED_OUT;
								break;

							case 30 ... 37: // writing color
								currentConsole->fg = parameter - 30;
								break;

							case 39: // reset foreground color
								currentConsole->fg = 7;
								break;

							case 40 ... 47: // screen color
								currentConsole->bg = parameter - 40;
								break;

							case 49: // reset background color
								currentConsole->fg = 0;
								break;
							}
						} while (escapelen > 0);

						escaping = false;
						break;

					default:
						// some sort of unsupported escape; just gloss over it
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

//---------------------------------------------------------------------------------
ssize_t debug_write(struct _reent *r, int fd, const char *ptr, size_t len) {
//---------------------------------------------------------------------------------
	svcOutputDebugString(ptr,len);
	return len;
}

static const devoptab_t dotab_3dmoo = {
	"3dmoo",
	0,
	NULL,
	NULL,
	debug_write,
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
PrintConsole* consoleInit(gfxScreen_t screen, PrintConsole* console) {
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

	gfxSetScreenFormat(screen,GSP_RGB565_OES);
	gfxSetDoubleBuffering(screen,false);
	gfxSwapBuffers();
	gspWaitForVBlank();

	console->frameBuffer = (u16*)gfxGetFramebuffer(screen, GFX_LEFT, NULL, NULL);

	if(screen==GFX_TOP) {
		console->consoleWidth = 50;
		console->windowWidth = 50;
	}


	consoleCls('2');

	return currentConsole;

}

//---------------------------------------------------------------------------------
void consoleDebugInit(debugDevice device){
//---------------------------------------------------------------------------------

	int buffertype = _IONBF;

	switch(device) {

	case debugDevice_3DMOO:
		devoptab_list[STD_ERR] = &dotab_3dmoo;
		buffertype = _IOLBF;
		break;
	case debugDevice_CONSOLE:
		devoptab_list[STD_ERR] = &dotab_stdout;
		break;
	case debugDevice_NULL:
		devoptab_list[STD_ERR] = &dotab_null;
		break;
	}
	setvbuf(stderr, NULL , buffertype, 0);

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
			u32 *from = (u32*)((int)src & ~3);
			u32 *to = (u32*)((int)dst & ~3);
			for (j=0;j<(((currentConsole->windowHeight-1)*8)/2);j++) *(to--) = *(from--);
			dst += 240;
			src += 240;
		}

		consoleClearLine('2');
		gfxFlushBuffers();
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

	if (currentConsole->flags & CONSOLE_COLOR_BOLD) {
		writingColor += 8;
	} else if (currentConsole->flags & CONSOLE_COLOR_FAINT) {
		writingColor += 16;
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

	if (currentConsole->flags & CONSOLE_UNDERLINE) b8 = 0xff;

	if (currentConsole->flags & CONSOLE_CROSSED_OUT) b4 = 0xff;

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
void consolePrintChar(int c) {
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
			gfxFlushBuffers();
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


