/**
 * @file errf.h
 * @brief Error Display API
 */

#pragma once

#include <3ds/types.h>

/// Used for register dumps.
typedef struct {
	u32 r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, cpsr;
} ERRF_ExceptionContext;

/// Types of errors that can be thrown by err:f.
typedef enum {
	ERRF_ERRTYPE_GENERIC    = 0, ///< For generic errors. Shows miscellaneous info.
	ERRF_ERRTYPE_EXCEPTION  = 3, ///< For exceptions, or more specifically 'crashes'. union data should be exception_data.
	ERRF_ERRTYPE_FAILURE    = 4, ///< For general failure. Shows a message. union data should have a string set in failure_mesg
	ERRF_ERRTYPE_LOGGED     = 5  ///< Outputs logs to NAND in some cases.
} ERRF_ErrType;

/// Types of 'Exceptions' thrown for ERRF_ERRTYPE_EXCEPTION
typedef enum {
	ERRF_EXCEPTION_PREFETCH_ABORT = 0, ///< Prefetch Abort
	ERRF_EXCEPTION_DATA_ABORT     = 1, ///< Data abort
	ERRF_EXCEPTION_UNDEFINED      = 2, ///< Undefined instruction
	ERRF_EXCEPTION_VFP            = 3  ///< VFP (floating point) exception.
} ERRF_ExceptionType;

typedef struct {
	ERRF_ExceptionType type; ///< Type of the exception. One of the ERRF_EXCEPTION_* values.
	u8  reserved[3];
	u32 reg1;                ///< If type is prefetch, this should be ifsr, and on data abort dfsr
	u32 reg2;                ///< If type is prefetch, this should be r15, and dfar on data abort
	u32 fpexc;
	u32 fpinst;
	u32 fpint2;
} ERRF_ExceptionInfo;

typedef struct {
	ERRF_ExceptionInfo excep;   ///< Exception info struct
	ERRF_ExceptionContext regs; ///< Register dump.
	u8 pad[4];
} ERRF_ExceptionData;

typedef struct {
	ERRF_ErrType type; ///< Type, one of the ERRF_ERRTYPE_* enum
	u8  revHigh;       ///< High revison ID
	u16 revLow;        ///< Low revision ID
	u32 resCode;       ///< Result code
	u32 pcAddr;        ///< PC address at exception
	u32 procId;        ///< Process ID.
	u64 titleId;       ///< Title ID.
	u64 appTitleId;    ///< Application Title ID.
	union {
		ERRF_ExceptionData exception_data; ///< Data for when type is ERRF_ERRTYPE_EXCEPTION
		char failure_mesg[60];             ///< String for when type is ERRF_ERRTYPE_FAILURE
	} data;                                ///< The different types of data for errors.
} ERRF_FatalErrInfo;

/// Initializes ERR:f.
Result errfInit(void);

/// Exits ERR:f.
void errfExit(void);

/**
 * @brief Gets the current err:f API session handle.
 * @return The current err:f API session handle.
 */
Handle *errfGetSessionHandle(void);

/**
 * @brief Throws a system error and possibly results in ErrDisp triggering. After performing this,
 * the system may panic and need to be rebooted. Extra information will be displayed on the
 * top screen with a developer console or the proper patches in a CFW applied.
 * @param error Error to throw.
 */
Result ERRF_Throw(ERRF_FatalErrInfo *error);
