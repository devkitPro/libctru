/**
 * @file swkbd.h
 * @brief Software keyboard applet.
 */
#pragma once
#include <3ds/types.h>

/// Keyboard types.
typedef enum
{
	SWKBD_TYPE_NORMAL = 0, ///< Normal keyboard with several pages (QWERTY/accents/symbol/mobile)
	SWKBD_TYPE_QWERTY,     ///< QWERTY keyboard only.
	SWKBD_TYPE_NUMPAD,     ///< Number pad.
	SWKBD_TYPE_WESTERN,    ///< On JPN systems, a text keyboard without Japanese input capabilities, otherwise same as SWKBD_TYPE_NORMAL.
} SwkbdType;

/// Accepted input types.
typedef enum
{
	SWKBD_ANYTHING = 0,      ///< All inputs are accepted.
	SWKBD_NOTEMPTY,          ///< Empty inputs are not accepted.
	SWKBD_NOTEMPTY_NOTBLANK, ///< Empty or blank inputs (consisting solely of whitespace) are not accepted.
	SWKBD_NOTBLANK_NOTEMPTY = SWKBD_NOTEMPTY_NOTBLANK,
	SWKBD_NOTBLANK,          ///< Blank inputs (consisting solely of whitespace) are not accepted, but empty inputs are.
	SWKBD_FIXEDLEN,          ///< The input must have a fixed length (specified by maxTextLength in swkbdInit).
} SwkbdValidInput;

/// Keyboard dialog buttons.
typedef enum
{
	SWKBD_BUTTON_LEFT = 0, ///< Left button (usually Cancel)
	SWKBD_BUTTON_MIDDLE,   ///< Middle button (usually I Forgot)
	SWKBD_BUTTON_RIGHT,    ///< Right button (usually OK)
	SWKBD_BUTTON_CONFIRM = SWKBD_BUTTON_RIGHT,
	SWKBD_BUTTON_NONE,     ///< No button (returned by swkbdInputText in special cases)
} SwkbdButton;

/// Keyboard password modes.
typedef enum
{
	SWKBD_PASSWORD_NONE = 0,   ///< Characters are not concealed.
	SWKBD_PASSWORD_HIDE,       ///< Characters are concealed immediately.
	SWKBD_PASSWORD_HIDE_DELAY, ///< Characters are concealed a second after they've been typed.
} SwkbdPasswordMode;

/// Keyboard input filtering flags.
enum
{
	SWKBD_FILTER_DIGITS    = BIT(0), ///< Disallow the use of more than a certain number of digits (0 or more)
	SWKBD_FILTER_AT        = BIT(1), ///< Disallow the use of the @ sign.
	SWKBD_FILTER_PERCENT   = BIT(2), ///< Disallow the use of the % sign.
	SWKBD_FILTER_BACKSLASH = BIT(3), ///< Disallow the use of the \ sign.
	SWKBD_FILTER_PROFANITY = BIT(4), ///< Disallow profanity using Nintendo's profanity filter.
	SWKBD_FILTER_CALLBACK  = BIT(5), ///< Use a callback in order to check the input.
};

/// Keyboard features.
enum
{
	SWKBD_PARENTAL          = BIT(0), ///< Parental PIN mode.
	SWKBD_DARKEN_TOP_SCREEN = BIT(1), ///< Darken the top screen when the keyboard is shown.
	SWKBD_PREDICTIVE_INPUT  = BIT(2), ///< Enable predictive input (necessary for Kanji input in JPN systems).
	SWKBD_MULTILINE         = BIT(3), ///< Enable multiline input.
	SWKBD_FIXED_WIDTH       = BIT(4), ///< Enable fixed-width mode.
	SWKBD_ALLOW_HOME        = BIT(5), ///< Allow the usage of the HOME button.
	SWKBD_ALLOW_RESET       = BIT(6), ///< Allow the usage of a software-reset combination.
	SWKBD_ALLOW_POWER       = BIT(7), ///< Allow the usage of the POWER button.
	SWKBD_DEFAULT_QWERTY    = BIT(9), ///< Default to the QWERTY page when the keyboard is shown.
};

/// Keyboard filter callback return values.
typedef enum
{
	SWKBD_CALLBACK_OK = 0,   ///< Specifies that the input is valid.
	SWKBD_CALLBACK_CLOSE,    ///< Displays an error message, then closes the keyboard.
	SWKBD_CALLBACK_CONTINUE, ///< Displays an error message and continues displaying the keyboard.
} SwkbdCallbackResult;

/// Keyboard return values.
typedef enum
{
	SWKBD_NONE          = -1, ///< Dummy/unused.
	SWKBD_INVALID_INPUT = -2, ///< Invalid parameters to swkbd.
	SWKBD_OUTOFMEM      = -3, ///< Out of memory.

	SWKBD_D0_CLICK = 0, ///< The button was clicked in 1-button dialogs.
	SWKBD_D1_CLICK0,    ///< The left button was clicked in 2-button dialogs.
	SWKBD_D1_CLICK1,    ///< The right button was clicked in 2-button dialogs.
	SWKBD_D2_CLICK0,    ///< The left button was clicked in 3-button dialogs.
	SWKBD_D2_CLICK1,    ///< The middle button was clicked in 3-button dialogs.
	SWKBD_D2_CLICK2,    ///< The right button was clicked in 3-button dialogs.

	SWKBD_HOMEPRESSED = 10, ///< The HOME button was pressed.
	SWKBD_RESETPRESSED,     ///< The soft-reset key combination was pressed.
	SWKBD_POWERPRESSED,     ///< The POWER button was pressed.

	SWKBD_PARENTAL_OK = 20, ///< The parental PIN was verified successfully.
	SWKBD_PARENTAL_FAIL,    ///< The parental PIN was incorrect.

	SWKBD_BANNED_INPUT = 30, ///< The filter callback returned SWKBD_CALLBACK_CLOSE.
} SwkbdResult;

/// Maximum dictionary word length, in UTF-16 code units.
#define SWKBD_MAX_WORD_LEN         40
/// Maximum button text length, in UTF-16 code units.
#define SWKBD_MAX_BUTTON_TEXT_LEN  16
/// Maximum hint text length, in UTF-16 code units.
#define SWKBD_MAX_HINT_TEXT_LEN    64
/// Maximum filter callback error message length, in UTF-16 code units.
#define SWKBD_MAX_CALLBACK_MSG_LEN 256

/// Keyboard dictionary word for predictive input.
typedef struct
{
	u16 reading[SWKBD_MAX_WORD_LEN+1]; ///< Reading of the word (that is, the string that needs to be typed).
	u16 word[SWKBD_MAX_WORD_LEN+1];    ///< Spelling of the word.
	u8 language;                       ///< Language the word applies to.
	bool all_languages;                ///< Specifies if the word applies to all languages.
} SwkbdDictWord;

/// Keyboard filter callback function.
typedef SwkbdCallbackResult (* SwkbdCallbackFn)(void* user, const char** ppMessage, const char* text, size_t textlen);
/// Keyboard status data.
typedef struct { u32 data[0x11]; } SwkbdStatusData;
/// Keyboard predictive input learning data.
typedef struct { u32 data[0x291B]; } SwkbdLearningData;

/// Internal libctru book-keeping structure for software keyboards.
typedef struct
{
	const char* initial_text;
	const SwkbdDictWord* dict;
	SwkbdStatusData* status_data;
	SwkbdLearningData* learning_data;
	SwkbdCallbackFn callback;
	void* callback_user;
} SwkbdExtra;

/// Software keyboard parameter structure, it shouldn't be modified directly.
typedef struct
{
	int type;
	int num_buttons_m1;
	int valid_input;
	int password_mode;
	int is_parental_screen;
	int darken_top_screen;
	u32 filter_flags;
	u32 save_state_flags;
	u16 max_text_len;
	u16 dict_word_count;
	u16 max_digits;
	u16 button_text[3][SWKBD_MAX_BUTTON_TEXT_LEN+1];
	u16 numpad_keys[2];
	u16 hint_text[SWKBD_MAX_HINT_TEXT_LEN+1];
	bool predictive_input;
	bool multiline;
	bool fixed_width;
	bool allow_home;
	bool allow_reset;
	bool allow_power;
	bool unknown; // XX: what is this supposed to do? "communicateWithOtherRegions"
	bool default_qwerty;
	bool button_submits_text[4];
	u16 language; // XX: not working? supposedly 0 = use system language, CFG_Language+1 = pick language
	int initial_text_offset;
	int dict_offset;
	int initial_status_offset;
	int initial_learning_offset;
	size_t shared_memory_size;
	u32 version;
	SwkbdResult result;
	int status_offset;
	int learning_offset;
	int text_offset;
	u16 text_length;
	int callback_result;
	u16 callback_msg[SWKBD_MAX_CALLBACK_MSG_LEN+1];
	bool skip_at_check;
	union
	{
		u8 reserved[171];
		SwkbdExtra extra;
	};
} SwkbdState;

/**
 * @brief Initializes software keyboard status.
 * @param swkbd Pointer to swkbd state.
 * @param type Keyboard type.
 * @param numButtons Number of dialog buttons to display (1, 2 or 3).
 * @param maxTextLength Maximum number of UTF-16 code units that input text can have (or -1 to let libctru use a big default).
 */
void swkbdInit(SwkbdState* swkbd, SwkbdType type, int numButtons, int maxTextLength);

/**
 * @brief Configures password mode in a software keyboard.
 * @param swkbd Pointer to swkbd state.
 * @param mode Password mode.
 */
static inline void swkbdSetPasswordMode(SwkbdState* swkbd, SwkbdPasswordMode mode)
{
	swkbd->password_mode = mode;
}

/**
 * @brief Configures input validation in a software keyboard.
 * @param swkbd Pointer to swkbd state.
 * @param validInput Specifies which inputs are valid.
 * @param filterFlags Bitmask specifying which characters are disallowed (filtered).
 * @param maxDigits In case digits are disallowed, specifies how many digits are allowed at maximum in input strings (0 completely restricts digit input).
 */
static inline void swkbdSetValidation(SwkbdState* swkbd, SwkbdValidInput validInput, u32 filterFlags, int maxDigits)
{
	swkbd->valid_input = validInput;
	swkbd->filter_flags = filterFlags;
	swkbd->max_digits = (filterFlags & SWKBD_FILTER_DIGITS) ? maxDigits : 0;
}

/**
 * @brief Configures what characters will the two bottom keys in a numpad produce.
 * @param swkbd Pointer to swkbd state.
 * @param left Unicode codepoint produced by the leftmost key in the bottom row (0 hides the key).
 * @param left Unicode codepoint produced by the rightmost key in the bottom row (0 hides the key).
 */
static inline void swkbdSetNumpadKeys(SwkbdState* swkbd, int left, int right)
{
	swkbd->numpad_keys[0] = left;
	swkbd->numpad_keys[1] = right;
}

/**
 * @brief Specifies which special features are enabled in a software keyboard.
 * @param swkbd Pointer to swkbd state.
 * @param features Feature bitmask.
 */
void swkbdSetFeatures(SwkbdState* swkbd, u32 features);

/**
 * @brief Sets the hint text of a software keyboard (that is, the help text that is displayed when the textbox is empty).
 * @param swkbd Pointer to swkbd state.
 * @param text Hint text.
 */
void swkbdSetHintText(SwkbdState* swkbd, const char* text);

/**
 * @brief Configures a dialog button in a software keyboard.
 * @param swkbd Pointer to swkbd state.
 * @param button Specifies which button to configure.
 * @param text Button text.
 * @param submit Specifies whether pushing the button will submit the text or discard it.
 */
void swkbdSetButton(SwkbdState* swkbd, SwkbdButton button, const char* text, bool submit);

/**
 * @brief Sets the initial text that a software keyboard will display on launch.
 * @param swkbd Pointer to swkbd state.
 * @param text Initial text.
 */
void swkbdSetInitialText(SwkbdState* swkbd, const char* text);

/**
 * @brief Configures a word in a predictive dictionary for use with a software keyboard.
 * @param word Pointer to dictionary word structure.
 * @param reading Reading of the word, that is, the sequence of characters that need to be typed to trigger the word in the predictive input system.
 * @param text Spelling of the word, that is, the actual characters that will be produced when the user decides to select the word.
 */
void swkbdSetDictWord(SwkbdDictWord* word, const char* reading, const char* text);

/**
 * @brief Sets the custom word dictionary to be used with the predictive input system of a software keyboard.
 * @param swkbd Pointer to swkbd state.
 * @param dict Pointer to dictionary words.
 * @param wordCount Number of words in the dictionary.
 */
void swkbdSetDictionary(SwkbdState* swkbd, const SwkbdDictWord* dict, int wordCount);

/**
 * @brief Configures software keyboard internal status management.
 * @param swkbd Pointer to swkbd state.
 * @param data Pointer to internal status structure (can be in, out or both depending on the other parameters).
 * @param in Specifies whether the data should be read from the structure when the keyboard is launched.
 * @param out Specifies whether the data should be written to the structure when the keyboard is closed.
 */
void swkbdSetStatusData(SwkbdState* swkbd, SwkbdStatusData* data, bool in, bool out);

/**
 * @brief Configures software keyboard predictive input learning data management.
 * @param swkbd Pointer to swkbd state.
 * @param data Pointer to learning data structure (can be in, out or both depending on the other parameters).
 * @param in Specifies whether the data should be read from the structure when the keyboard is launched.
 * @param out Specifies whether the data should be written to the structure when the keyboard is closed.
 */
void swkbdSetLearningData(SwkbdState* swkbd, SwkbdLearningData* data, bool in, bool out);

/**
 * @brief Configures a custom function to be used to check the validity of input when it is submitted in a software keyboard.
 * @param swkbd Pointer to swkbd state.
 * @param callback Filter callback function.
 * @param user Custom data to be passed to the callback function.
 */
void swkbdSetFilterCallback(SwkbdState* swkbd, SwkbdCallbackFn callback, void* user);

/**
 * @brief Launches a software keyboard in order to input text.
 * @param swkbd Pointer to swkbd state.
 * @param buf Pointer to output buffer which will hold the inputted text.
 * @param bufsize Maximum number of UTF-8 code units that the buffer can hold (including null terminator).
 * @return The identifier of the dialog button that was pressed, or SWKBD_BUTTON_NONE if a different condition was triggered - in that case use swkbdGetResult to check the condition.
 */
SwkbdButton swkbdInputText(SwkbdState* swkbd, char* buf, size_t bufsize);

/**
 * @brief Retrieves the result condition of a software keyboard after it has been used.
 * @param swkbd Pointer to swkbd state.
 * @return The result value.
 */
static inline SwkbdResult swkbdGetResult(SwkbdState* swkbd)
{
	return swkbd->result;
}
