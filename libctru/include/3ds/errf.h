/**
 * @file errf.h
 * @brief Error Display API
 */

#pragma once

#include <3ds/types.h>

/// Types of errors that can be thrown by err:f.
typedef enum {
	ERRF_ERRTYPE_GENERIC      = 0,  ///< Generic fatal error. Shows miscellaneous info, including the address of the caller
	ERRF_ERRTYPE_NAND_DAMAGED = 1,  ///< Damaged NAND (CC_ERROR after reading CSR)
	ERRF_ERRTYPE_CARD_REMOVED = 2,  ///< Game content storage medium (cartridge and/or SD card) ejected. Not logged
	ERRF_ERRTYPE_EXCEPTION    = 3,  ///< CPU or VFP exception
	ERRF_ERRTYPE_FAILURE      = 4,  ///< Fatal error with a message instead of the caller's address
	ERRF_ERRTYPE_LOG_ONLY     = 5,  ///< Log-level failure. Does not display the exception and does not force the system to reboot
} ERRF_ErrType;

/// Types of 'Exceptions' thrown for ERRF_ERRTYPE_EXCEPTION
typedef enum {
	ERRF_EXCEPTION_PREFETCH_ABORT = 0, ///< Prefetch Abort
	ERRF_EXCEPTION_DATA_ABORT     = 1, ///< Data abort
	ERRF_EXCEPTION_UNDEFINED      = 2, ///< Undefined instruction
	ERRF_EXCEPTION_VFP            = 3, ///< VFP (floating point) exception.
} ERRF_ExceptionType;

typedef struct {
	ERRF_ExceptionType type; ///< Type of the exception. One of the ERRF_EXCEPTION_* values.
	u8  reserved[3];
	u32 fsr;                ///< ifsr (prefetch abort) / dfsr (data abort)
	u32 far;                ///< pc = ifar (prefetch abort) / dfar (data abort)
	u32 fpexc;
	u32 fpinst;
	u32 fpinst2;
} ERRF_ExceptionInfo;

typedef struct {
	ERRF_ExceptionInfo excep;   ///< Exception info struct
	CpuRegisters regs;          ///< CPU register dump.
} ERRF_ExceptionData;

typedef struct {
	ERRF_ErrType type; ///< Type, one of the ERRF_ERRTYPE_* enum
	u8  revHigh;       ///< High revison ID
	u16 revLow;        ///< Low revision ID
	u32 resCode;       ///< Result code
	u32 pcAddr;        ///< PC address at exception
	u32 procId;        ///< Process ID of the caller
	u64 titleId;       ///< Title ID of the caller
	u64 appTitleId;    ///< Title ID of the running application
	union {
		ERRF_ExceptionData exception_data; ///< Data for when type is ERRF_ERRTYPE_EXCEPTION
		char failure_mesg[0x60];           ///< String for when type is ERRF_ERRTYPE_FAILURE
	} data;                                ///< The different types of data for errors.
} ERRF_FatalErrInfo;

/// Initializes ERR:f. Unless you plan to call ERRF_Throw yourself, do not use this.
Result errfInit(void);

/// Exits ERR:f. Unless you plan to call ERRF_Throw yourself, do not use this.
void errfExit(void);

/**
 * @brief Gets the current err:f API session handle.
 * @return The current err:f API session handle.
 */
Handle *errfGetSessionHandle(void);

/**
 * @brief Throws a system error and possibly logs it.
 * @param[in] error Error to throw.
 *
 * ErrDisp may convert the error info to \ref ERRF_ERRTYPE_NAND_DAMAGED or \ref ERRF_ERRTYPE_CARD_REMOVED
 * depending on the error code.
 *
 * Except with \ref ERRF_ERRTYPE_LOG_ONLY, the system will panic and will need to be rebooted.
 * Fatal error information will also be logged into a file, unless the type either \ref ERRF_ERRTYPE_NAND_DAMAGED
 * or \ref ERRF_ERRTYPE_CARD_REMOVED.
 *
 * No error will be shown if the system is asleep.
 *
 * On retail units with vanilla firmware, no detailed information will be displayed on screen.
 *
 * You may wish to use ERRF_ThrowResult() or ERRF_ThrowResultWithMessage() instead of
 * constructing the ERRF_FatalErrInfo struct yourself.
 */
Result ERRF_Throw(const ERRF_FatalErrInfo* error);

/**
 * @brief Throws (and logs) a system error with the given Result code.
 * @param[in] failure Result code to throw.
 *
 * This calls \ref ERRF_Throw with error type \ref ERRF_ERRTYPE_GENERIC and fills in the required data.
 *
 * This function \em does fill in the address where this function was called from.
 */
Result ERRF_ThrowResult(Result failure);

/**
 * @brief Logs a system error with the given Result code.
 * @param[in] failure Result code to log.
 *
 * Similar to \ref ERRF_Throw, except that it does not display anything on the screen,
 * nor does it force the system to reboot.
 *
 * This function \em does fill in the address where this function was called from.
 */
Result ERRF_LogResult(Result failure);

/**
 * @brief Throws a system error with the given Result code and message.
 * @param[in] failure Result code to throw.
 * @param[in] message The message to display.
 *
 * This calls \ref ERRF_Throw with error type \ref ERRF_ERRTYPE_FAILURE and fills in the required data.
 *
 * This function does \em not fill in the address where this function was called from because it
 * would not be displayed.
 */
Result ERRF_ThrowResultWithMessage(Result failure, const char* message);

/**
 * @brief Specify an additional user string to use for error reporting.
 * @param[in] user_string User string (up to 256 bytes, not including NUL byte)
 */
Result ERRF_SetUserString(const char* user_string);

/**
 * @brief Handles an exception using ErrDisp.
 * @param excep Exception information
 * @param regs CPU registers
 *
 * You might want to clear ENVINFO's bit0 to be able to see any debugging information.
 * @sa threadOnException
 */
void ERRF_ExceptionHandler(ERRF_ExceptionInfo* excep, CpuRegisters* regs) __attribute__((noreturn));
