/**
 * @file error.h
 * @brief Error applet.
 */
#pragma once
#include <3ds/types.h>

 enum
{
	ERROR_LANGUAGE_FLAG = 0x100,    ///<??-Unknown flag
	ERROR_WORD_WRAP_FLAG = 0x200    ///<??-Unknown flag
};


///< Type of Error applet to be called

typedef enum 
{
	ERROR_CODE = 0,						///< Displays the infrastructure communications-related error message corresponding to the error code.
	ERROR_TEXT,						///< Displays text passed to this applet.
	ERROR_EULA,						///< Displays the EULA
	ERROR_TYPE_EULA_FIRST_BOOT,				///< Use prohibited.
	ERROR_TYPE_EULA_DRAW_ONLY,				///< Use prohibited.
	ERROR_TYPE_AGREE,					///< Use prohibited.
	ERROR_CODE_LANGUAGE = ERROR_CODE | ERROR_LANGUAGE_FLAG,	///< Displays a network error message in a specified language.
	ERROR_TEXT_LANGUAGE = ERROR_TEXT | ERROR_LANGUAGE_FLAG,	///< Displays text passed to this applet in a specified language.
	ERROR_EULA_LANGUAGE = ERROR_EULA | ERROR_LANGUAGE_FLAG,	///< Displays EULA in a specified language.
	ERROR_TEXT_WORD_WRAP = ERROR_TEXT | ERROR_WORD_WRAP_FLAG,///< Displays the custom error message passed to this applet with automatic line wrapping
	ERROR_TEXT_LANGUAGE_WORD_WRAP = ERROR_TEXT | ERROR_LANGUAGE_FLAG | ERROR_WORD_WRAP_FLAG	///< Displays the custom error message with automatic line wrapping and in the specified language.
}errorType;

///< Flags for the Upper Screen.Does nothing even if specified.

typedef enum
{
	ERROR_NORMAL = 0,    
	ERROR_STEREO
}errorScreenFlag;

///< Return code of the Error module.Use UNKNOWN for simple apps.

typedef enum 
{
	ERROR_UNKNOWN = -1,       
	ERROR_NONE    = 0,        
	ERROR_SUCCESS,            
	ERROR_NOT_SUPPORTED,      
	ERROR_HOME_BUTTON = 10,   
	ERROR_SOFTWARE_RESET,     
	ERROR_POWER_BUTTON     
}errorReturnCode;

///< Structure to be passed to the applet.Shouldn't be modified directly.

typedef struct 
{
	errorType type;
	int errorCode;
	errorScreenFlag upperScreenFlag;
	u16 useLanguage;
	u16 Text[1900];
	bool homeButton;
	bool softwareReset;
	bool appJump;
	errorReturnCode returnCode;
	u16 eulaVersion;
}errorConf;
/**
* @brief Init the error applet.
* @param err Pointer to errorConf.
* @param type errorType Type of error.
* @param lang CFG_Language Lang of error. 
*/
void errorInit(errorConf* err, errorType type, CFG_Language lang);
/**
* @brief Sets error code to display.
* @param err Pointer to errorConf.
* @param error Error-code to display.
*/
void errorCode(errorConf* err, int error);
/**
* @brief Sets error text to display.
* @param err Pointer to errorConf.
* @param text Error-text to display.
*/
void errorText(errorConf* err, const char* text);
/**
* @brief Displays the error applet.
* @param err Pointer to errorConf.
*/
void errorDisp(errorConf* err);
