/**
 * @file result.h
 * @brief 3DS result code tools
 */
#pragma once
#include "types.h"

/// Checks whether a result code indicates success.
#define R_SUCCEEDED(res)   ((res)>=0)
/// Checks whether a result code indicates failure.
#define R_FAILED(res)      ((res)<0)
/// Returns the level of a result code.
#define R_LEVEL(res)       (((res)>>27)&0x1F)
/// Returns the summary of a result code.
#define R_SUMMARY(res)     (((res)>>21)&0x3F)
/// Returns the module ID of a result code.
#define R_MODULE(res)      (((res)>>10)&0xFF)
/// Returns the description of a result code.
#define R_DESCRIPTION(res) ((res)&0x3FF)

/// Builds a result code from its constituent components.
#define MAKERESULT(level,summary,module,description) \
	((((level)&0x1F)<<27) | (((summary)&0x3F)<<21) | (((module)&0xFF)<<10) | ((description)&0x3FF))

/// Result code level values.
enum
{
	// >= 0
	RL_SUCCESS = 0,
	RL_INFO    = 1,

	// < 0
	RL_FATAL        = 0x1F,
	RL_RESET        = RL_FATAL - 1,
	RL_REINITIALIZE = RL_FATAL - 2,
	RL_USAGE        = RL_FATAL - 3,
	RL_PERMANENT    = RL_FATAL - 4,
	RL_TEMPORARY    = RL_FATAL - 5,
	RL_STATUS       = RL_FATAL - 6,
};

/// Result code summary values.
enum
{
	RS_SUCCESS       = 0,
	RS_NOP           = 1,
	RS_WOULDBLOCK    = 2,
	RS_OUTOFRESOURCE = 3,
	RS_NOTFOUND      = 4,
	RS_INVALIDSTATE  = 5,
	RS_NOTSUPPORTED  = 6,
	RS_INVALIDARG    = 7,
	RS_WRONGARG      = 8,
	RS_CANCELED      = 9,
	RS_STATUSCHANGED = 10,
	RS_INTERNAL      = 11,
	RS_INVALIDRESVAL = 63,
};

/// Result code generic description values.
enum
{
	RD_SUCCESS              = 0,
	RD_INVALID_RESULT_VALUE = 0x3FF,
	RD_TIMEOUT              = RD_INVALID_RESULT_VALUE -  1,
	RD_OUT_OF_RANGE         = RD_INVALID_RESULT_VALUE -  2,
	RD_ALREADY_EXISTS       = RD_INVALID_RESULT_VALUE -  3,
	RD_CANCEL_REQUESTED     = RD_INVALID_RESULT_VALUE -  4,
	RD_NOT_FOUND            = RD_INVALID_RESULT_VALUE -  5,
	RD_ALREADY_INITIALIZED  = RD_INVALID_RESULT_VALUE -  6,
	RD_NOT_INITIALIZED      = RD_INVALID_RESULT_VALUE -  7,
	RD_INVALID_HANDLE       = RD_INVALID_RESULT_VALUE -  8,
	RD_INVALID_POINTER      = RD_INVALID_RESULT_VALUE -  9,
	RD_INVALID_ADDRESS      = RD_INVALID_RESULT_VALUE - 10,
	RD_NOT_IMPLEMENTED      = RD_INVALID_RESULT_VALUE - 11,
	RD_OUT_OF_MEMORY        = RD_INVALID_RESULT_VALUE - 12,
	RD_MISALIGNED_SIZE      = RD_INVALID_RESULT_VALUE - 13,
	RD_MISALIGNED_ADDRESS   = RD_INVALID_RESULT_VALUE - 14,
	RD_BUSY                 = RD_INVALID_RESULT_VALUE - 15,
	RD_NO_DATA              = RD_INVALID_RESULT_VALUE - 16,
	RD_INVALID_COMBINATION  = RD_INVALID_RESULT_VALUE - 17,
	RD_INVALID_ENUM_VALUE   = RD_INVALID_RESULT_VALUE - 18,
	RD_INVALID_SIZE         = RD_INVALID_RESULT_VALUE - 19,
	RD_ALREADY_DONE         = RD_INVALID_RESULT_VALUE - 20,
	RD_NOT_AUTHORIZED       = RD_INVALID_RESULT_VALUE - 21,
	RD_TOO_LARGE            = RD_INVALID_RESULT_VALUE - 22,
	RD_INVALID_SELECTION    = RD_INVALID_RESULT_VALUE - 23,
};
