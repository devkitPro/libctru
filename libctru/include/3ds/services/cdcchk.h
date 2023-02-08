/**
 * @file cdcchk.h
 * @brief CODEC Hardware Check service.
 */
#pragma once

#include <3ds/types.h>

/// I2S line enumeration
typedef enum {
    CODEC_I2S_LINE_1,     ///< Primary I2S line, used by DSP/Mic (configurable)/GBA sound controller.
    CODEC_I2S_LINE_2,     ///< Secondary I2S line, used by CSND hardware.
} CodecI2sLine;

/// Initializes CDCCHK.
Result cdcChkInit(void);

/// Exits CDCCHK.
void cdcChkExit(void);

/**
 * @brief Gets a pointer to the current cdc:CHK session handle.
 * @return A pointer to the current cdc:CHK session handle.
 */
Handle *cdcChkGetSessionHandle(void);

/**
 * @brief Reads multiple registers from the CODEC, using the old
 * SPI hardware interface and a 4MHz baudrate.
 * @param pageId CODEC Page ID.
 * @param initialRegAddr Address of the CODEC register to start with.
 * @param[out] outData Where to write the read data to.
 * @param size Number of registers to read (bytes to read, max. 64).
 */
Result CDCCHK_ReadRegisters1(u8 pageId, u8 initialRegAddr, void *outData, size_t size);

/**
 * @brief Reads multiple registers from the CODEC, using the new
 * SPI hardware interface and a 16MHz baudrate.
 * @param pageId CODEC Page ID.
 * @param initialRegAddr Address of the CODEC register to start with.
 * @param[out] outData Where to read the data to.
 * @param size Number of registers to read (bytes to read, max. 64).
 */
Result CDCCHK_ReadRegisters2(u8 pageId, u8 initialRegAddr, void *outData, size_t size);

/**
 * @brief Writes multiple registers to the CODEC, using the old
 * SPI hardware interface and a 4MHz baudrate.
 * @param pageId CODEC Page ID.
 * @param initialRegAddr Address of the CODEC register to start with.
 * @param data Where to read the data to write from.
 * @param size Number of registers to write (bytes to read, max. 64).
 */
Result CDCCHK_WriteRegisters1(u8 pageId, u8 initialRegAddr, const void *data, size_t size);

/**
 * @brief Writes multiple registers to the CODEC, using the new
 * SPI hardware interface and a 16MHz baudrate.
 * @param pageId CODEC Page ID.
 * @param initialRegAddr Address of the CODEC register to start with.
 * @param data Where to read the data to write from.
 * @param size Number of registers to write (bytes to read, max. 64).
 */
Result CDCCHK_WriteRegisters2(u8 pageId, u8 initialRegAddr, const void *data, size_t size);

/**
 * @brief Reads a single register from the NTR PMIC.
 * @param[out] outData Where to read the data to (1 byte).
 * @param regAddr Register address.
 * @note The NTR PMIC is emulated by the CODEC hardware and sends
 * IRQs to the MCU when relevant.
 */
Result CDCCHK_ReadNtrPmicRegister(u8 *outData, u8 regAddr);

/**
 * @brief Writes a single register from the NTR PMIC.
 * @param regAddr Register address.
 * @param data Data to write (1 byte).
 * @note The NTR PMIC is emulated by the CODEC hardware and sends
 * IRQs to the MCU when relevant.
 */
Result CDCCHK_WriteNtrPmicRegister(u8 regAddr, u8 data);

/** 
 * @brief Sets the DAC volume level for the specified I2S line.
 * @param i2sLine I2S line to set the volume for.
 * @param volume Volume level (-128 to 0).
*/
Result CDCCHK_SetI2sVolume(CodecI2sLine i2sLine, s8 volume);
