

/*! \file console.h
    \brief 3ds stdio support.

<div class="fileHeader">
Provides stdio integration for printing to the 3DS screen as well as debug print
functionality provided by stderr.

General usage is to initialize the console by:
consoleDemoInit()
or to customize the console usage by:
consoleInit()

*/

#ifndef CONSOLE_H
#define CONSOLE_H

#include <3ds/types.h>
#include <3ds/gfx.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef bool(* ConsolePrint)(void* con, int c);

//! a font struct for the console.
typedef struct ConsoleFont
{
	u8* gfx;			//!< A pointer to the font graphics
	u16 asciiOffset;	//!<  Offset to the first valid character in the font table
	u16 numChars;		//!< Number of characters in the font graphics

}ConsoleFont;

/** \brief console structure used to store the state of a console render context.

Default values from consoleGetDefault();
<div class="fixedFont"><pre>
PrintConsole defaultConsole =
{
	//Font:
	{
		(u8*)default_font_bin, //font gfx
		0, //first ascii character in the set
		128, //number of characters in the font set
	},
	0,0, //cursorX cursorY
	0,0, //prevcursorX prevcursorY
	40, //console width
	30, //console height
	0,  //window x
	0,  //window y
	32, //window width
	24, //window height
	3, //tab size
	0, //font character offset
	0,  //print callback
	false //console initialized
};
</pre></div>
*/
typedef struct PrintConsole
{
	ConsoleFont font;	//!< font of the console.

	u16 *frameBuffer;	//!< framebuffer address.

	int cursorX;		/*!< Current X location of the cursor (as a tile offset by default) */
	int cursorY;		/*!< Current Y location of the cursor (as a tile offset by default) */

	int prevCursorX;	/*!< Internal state */
	int prevCursorY;	/*!< Internal state */

	int consoleWidth;	/*!< Width of the console hardware layer in characters */
	int consoleHeight;	/*!< Height of the console hardware layer in characters  */

	int windowX;		/*!< Window X location in characters (not implemented) */
	int windowY;		/*!< Window Y location in characters (not implemented) */
	int windowWidth;	/*!< Window width in characters (not implemented) */
	int windowHeight;	/*!< Window height in characters (not implemented) */

	int tabSize;		/*!< Size of a tab*/
	int fg;				/*!< foreground color*/
	int bg;				/*!< background color*/
	int flags;			/*!< reverse/bright flags*/

	ConsolePrint PrintChar;			/*!< callback for printing a character. Should return true if it has handled rendering the graphics
									(else the print engine will attempt to render via tiles) */

	bool consoleInitialised;	/*!< True if the console is initialized */
}PrintConsole;

#define CONSOLE_COLOR_BOLD	(1<<0)
#define CONSOLE_COLOR_FAINT	(1<<1)
#define CONSOLE_ITALIC		(1<<2)
#define CONSOLE_UNDERLINE	(1<<3)
#define CONSOLE_BLINK_SLOW	(1<<4)
#define CONSOLE_BLINK_FAST	(1<<5)
#define CONSOLE_COLOR_REVERSE	(1<<6)
#define CONSOLE_CONCEAL		(1<<7)
#define CONSOLE_CROSSED_OUT	(1<<8)

//! Console debug devices supported by libnds.
typedef enum {
	debugDevice_NULL,	//!< swallows prints to stderr
	debugDevice_3DMOO,	//!< Directs stderr debug statements to 3dmoo
	debugDevice_CONSOLE,	//!< Directs stderr debug statements to 3DS console window
} debugDevice;

/*!	\brief Loads the font into the console
	\param console pointer to the console to update, if NULL it will update the current console
	\param font the font to load
*/
void consoleSetFont(PrintConsole* console, ConsoleFont* font);

/*!	\brief Sets the print window
	\param console console to set, if NULL it will set the current console window
	\param x x location of the window
	\param y y location of the window
	\param width width of the window
	\param height height of the window
*/
void consoleSetWindow(PrintConsole* console, int x, int y, int width, int height);

/*!	\brief Gets a pointer to the console with the default values
	this should only be used when using a single console or without changing the console that is returned, other wise use consoleInit()
	\return A pointer to the console with the default values
*/
PrintConsole* consoleGetDefault(void);

/*!	\brief Make the specified console the render target
	\param console A pointer to the console struct (must have been initialized with consoleInit(PrintConsole* console)
	\return a pointer to the previous console
*/
PrintConsole *consoleSelect(PrintConsole* console);

/*!	\brief Initialise the console.
	\param screen The screen to use for the console
	\param console A pointer to the console data to initialze (if it's NULL, the default console will be used)
	\return A pointer to the current console.
*/
PrintConsole* consoleInit(gfxScreen_t screen, PrintConsole* console);

/*!	\brief Initializes debug console output on stderr to the specified device
	\param device The debug device (or devices) to output debug print statements to
*/
void consoleDebugInit(debugDevice device);


//! Clears the screan by using iprintf("\x1b[2J");
void consoleClear(void);

#ifdef __cplusplus
}
#endif

#endif
