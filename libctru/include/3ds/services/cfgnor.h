/**
 * @file cfgnor.h
 * @brief CFGNOR service.
 */
#pragma once

/**
 * @brief Initializes CFGNOR.
 * @param value Unknown, usually 1.
 */
Result CFGNOR_Initialize(u8 value);

/// Shuts down CFGNOR.
Result CFGNOR_Shutdown();

/**
 * @brief Reads data from NOR.
 * @param offset Offset to read from.
 * @param buf Buffer to read data to.
 * @param size Size of the buffer.
 */
Result CFGNOR_ReadData(u32 offset, u32 *buf, u32 size);

/**
 * @brief Writes data to NOR.
 * @param offset Offset to write to.
 * @param buf Buffer to write data from.
 * @param size Size of the buffer.
 */
Result CFGNOR_WriteData(u32 offset, u32 *buf, u32 size);

/**
 * @brief Dumps the NOR flash.
 * @param buf Buffer to dump to.
 * @param size Size of the buffer.
 */
Result CFGNOR_DumpFlash(u32 *buf, u32 size);

/**
 * @brief Writes the NOR flash.
 * @param buf Buffer to write from.
 * @param size Size of the buffer.
 */
Result CFGNOR_WriteFlash(u32 *buf, u32 size);
