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

/// Result code module values.
enum
{
	RM_COMMON        = 0,
	RM_KERNEL        = 1,
	RM_UTIL          = 2,
	RM_FILE_SERVER   = 3,
	RM_LOADER_SERVER = 4,
	RM_TCB           = 5,
	RM_OS            = 6,
	RM_DBG           = 7,
	RM_DMNT          = 8,
	RM_PDN           = 9,
	RM_GSP           = 10,
	RM_I2C           = 11,
	RM_GPIO          = 12,
	RM_DD            = 13,
	RM_CODEC         = 14,
	RM_SPI           = 15,
	RM_PXI           = 16,
	RM_FS            = 17,
	RM_DI            = 18,
	RM_HID           = 19,
	RM_CAM           = 20,
	RM_PI            = 21,
	RM_PM            = 22,
	RM_PM_LOW        = 23,
	RM_FSI           = 24,
	RM_SRV           = 25,
	RM_NDM           = 26,
	RM_NWM           = 27,
	RM_SOC           = 28,
	RM_LDR           = 29,
	RM_ACC           = 30,
	RM_ROMFS         = 31,
	RM_AM            = 32,
	RM_HIO           = 33,
	RM_UPDATER       = 34,
	RM_MIC           = 35,
	RM_FND           = 36,
	RM_MP            = 37,
	RM_MPWL          = 38,
	RM_AC            = 39,
	RM_HTTP          = 40,
	RM_DSP           = 41,
	RM_SND           = 42,
	RM_DLP           = 43,
	RM_HIO_LOW       = 44,
	RM_CSND          = 45,
	RM_SSL           = 46,
	RM_AM_LOW        = 47,
	RM_NEX           = 48,
	RM_FRIENDS       = 49,
	RM_RDT           = 50,
	RM_APPLET        = 51,
	RM_NIM           = 52,
	RM_PTM           = 53,
	RM_MIDI          = 54,
	RM_MC            = 55,
	RM_SWC           = 56,
	RM_FATFS         = 57,
	RM_NGC           = 58,
	RM_CARD          = 59,
	RM_CARDNOR       = 60,
	RM_SDMC          = 61,
	RM_BOSS          = 62,
	RM_DBM           = 63,
	RM_CONFIG        = 64,
	RM_PS            = 65,
	RM_CEC           = 66,
	RM_IR            = 67,
	RM_UDS           = 68,
	RM_PL            = 69,
	RM_CUP           = 70,
	RM_GYROSCOPE     = 71,
	RM_MCU           = 72,
	RM_NS            = 73,
	RM_NEWS          = 74,
	RM_RO            = 75,
	RM_GD            = 76,
	RM_CARD_SPI      = 77,
	RM_EC            = 78,
	RM_WEB_BROWSER   = 79,
	RM_TEST          = 80,
	RM_ENC           = 81,
	RM_PIA           = 82,
	RM_ACT           = 83,
	RM_VCTL          = 84,
	RM_OLV           = 85,
	RM_NEIA          = 86,
	RM_NPNS          = 87,
	RM_AVD           = 90,
	RM_L2B           = 91,
	RM_MVD           = 92,
	RM_NFC           = 93,
	RM_UART          = 94,
	RM_SPM           = 95,
	RM_QTM           = 96,
	RM_NFP           = 97,
	RM_APPLICATION   = 254,
	RM_INVALIDRESVAL = 255,
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
