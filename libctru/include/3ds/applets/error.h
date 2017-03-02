/**
 * @file error.h
 * @brief Error applet.
 */
#pragma once
 enum
{
	ERROR_LANGUAGE_FLAG = 0x100,    ///<??-Unknown flag
	ERROR_WORD_WRAP_FLAG = 0x200    ///<??-Unknown flag
};


///< Type of Error applet to be called

typedef enum 
{
	ERROR_CODE = 0,                                             ///< Displays the infrastructure communications-related error message corresponding to the error code.
	ERROR_TEXT,                                                 ///< Displays text passed to this applet.
	EULA,                                                       ///< Displays the EULA
	ERROR_TYPE_EULA_FIRST_BOOT,                                 ///< Use prohibited.
	ERROR_TYPE_EULA_DRAW_ONLY,                                  ///< Use prohibited.
	ERROR_TYPE_AGREE,                                           ///< Use prohibited.
	ERROR_CODE_LANGUAGE = ERROR_CODE | ERROR_LANGUAGE_FLAG,     ///< Displays a network error message in a specified language.
	ERROR_TEXT_LANGUAGE = ERROR_TEXT | ERROR_LANGUAGE_FLAG,     ///< Displays text passed to this applet in a specified language.
	EULA_LANGUAGE = EULA | ERROR_LANGUAGE_FLAG,                 ///< Displays EULA in a specified language.
	ERROR_TEXT_WORD_WRAP = ERROR_TEXT | ERROR_WORD_WRAP_FLAG,   ///< Displays the custom error message passed to this applet with automatic line wrapping
	ERROR_TEXT_LANGUAGE_WORD_WRAP = ERROR_TEXT | ERROR_LANGUAGE_FLAG | ERROR_WORD_WRAP_FLAG,   ///< Displays the custom error message with automatic line wrapping and in the specified language.
}ErrorType;


///< Flags for the Upper Screen.Does nothing even if specified.

typedef enum
{
	NORMAL = 0,    
	STEREO,        
}ScreenFlag;


///< The language to use.Use DEFAULT if you want your System Setting language.

typedef enum
{
	DEFAULT = 0,   
	JAPANESE,      
	ENGLISH,       
	FRENCH,        
	GERMAN,        
	ITALIAN,       
	SPANISH,       
	SIMP_CHINESE,  
	KOREAN,        
	DUTCH,         
	PORTUGUESE,    
	RUSSIAN,       
	TRAD_CHINESE,  
}ErrorLang;


///< Return code of the Error module.Use UNKNOWN for simple apps.

typedef enum 
{
	UNKNOWN = -1,       
	NONE    = 0,        
	SUCCESS,            
	NOT_SUPPORTED,      
	HOME_BUTTON = 10,   
	SOFTWARE_RESET,     
	POWER_BUTTON,       
}ReturnCode;

///< Structure to be passed to the applet.Shouldn't be modified directly.

typedef struct 
{
	ErrorType errorType;
	int errorCode;
	ScreenFlag upperScreenFlag;
	ErrorLang useLanguage;
	u16 Text[1900];
	bool homeButton;
	bool softwareReset;
	bool appJump;
	ReturnCode returnCode;
	u16 eulaVersion;
}ErrConf;
/**
* @brief Init the error applet.
* @param err Pointer to ErrConf.
* @param type ErrorType Type of error.
* @param lang ErrorLang Lang of error. 
*/
void error_Init(ErrConf* err, ErrorType type, ErrorLang lang);
/**
* @brief Sets error code to display.
* @param err Pointer to ErrConf.
* @param error Error-code to display.
*/
void error_code(ErrConf* err, int error);
/**
* @brief Sets error text to display.
* @param err Pointer to ErrConf.
* @param text Error-text to display.
*/
void error_text(ErrConf* err, char* text);
/**
* @brief Displays the error applet.
* @param err Pointer to ErrConf.
*/
void error_disp(ErrConf* err);
