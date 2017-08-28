/**
 * @file gpu.h
 * @brief Barebones GPU communications driver.
 */
#pragma once

#include "registers.h"
#include "enums.h"

/// Creates a GPU command header from its write increments, mask, and register.
#define GPUCMD_HEADER(incremental, mask, reg) (((incremental)<<31)|(((mask)&0xF)<<16)|((reg)&0x3FF))

extern u32* gpuCmdBuf;      ///< GPU command buffer.
extern u32 gpuCmdBufSize;   ///< GPU command buffer size.
extern u32 gpuCmdBufOffset; ///< GPU command buffer offset.

/**
 * @brief Sets the GPU command buffer to use.
 * @param adr Pointer to the command buffer.
 * @param size Size of the command buffer.
 * @param offset Offset of the command buffer.
 */
static inline void GPUCMD_SetBuffer(u32* adr, u32 size, u32 offset)
{
	gpuCmdBuf=adr;
	gpuCmdBufSize=size;
	gpuCmdBufOffset=offset;
}

/**
 * @brief Sets the offset of the GPU command buffer.
 * @param offset Offset of the command buffer.
 */
static inline void GPUCMD_SetBufferOffset(u32 offset)
{
	gpuCmdBufOffset=offset;
}

/**
 * @brief Gets the current GPU command buffer.
 * @param addr Pointer to output the command buffer to.
 * @param size Pointer to output the size (in words) of the command buffer to.
 * @param offset Pointer to output the offset of the command buffer to.
 */
static inline void GPUCMD_GetBuffer(u32** addr, u32* size, u32* offset)
{
	if(addr)*addr=gpuCmdBuf;
	if(size)*size=gpuCmdBufSize;
	if(offset)*offset=gpuCmdBufOffset;
}

/**
 * @brief Adds raw GPU commands to the current command buffer.
 * @param cmd Buffer containing commands to add.
 * @param size Size of the buffer.
 */
void GPUCMD_AddRawCommands(const u32* cmd, u32 size);

/**
 * @brief Adds a GPU command to the current command buffer.
 * @param header Header of the command.
 * @param param Parameters of the command.
 * @param paramlength Size of the parameter buffer.
 */
void GPUCMD_Add(u32 header, const u32* param, u32 paramlength);

/**
 * @brief Splits the current GPU command buffer.
 * @param addr Pointer to output the command buffer to.
 * @param size Pointer to output the size (in words) of the command buffer to.
 */
void GPUCMD_Split(u32** addr, u32* size);

/**
 * @brief Converts a 32-bit float to a 16-bit float.
 * @param f Float to convert.
 * @return The converted float.
 */
u32 f32tof16(float f);

/**
 * @brief Converts a 32-bit float to a 20-bit float.
 * @param f Float to convert.
 * @return The converted float.
 */
u32 f32tof20(float f);

/**
 * @brief Converts a 32-bit float to a 24-bit float.
 * @param f Float to convert.
 * @return The converted float.
 */
u32 f32tof24(float f);

/**
 * @brief Converts a 32-bit float to a 31-bit float.
 * @param f Float to convert.
 * @return The converted float.
 */
u32 f32tof31(float f);

/// Adds a command with a single parameter to the current command buffer.
static inline void GPUCMD_AddSingleParam(u32 header, u32 param)
{
	GPUCMD_Add(header, &param, 1);
}

/// Adds a masked register write to the current command buffer.
#define GPUCMD_AddMaskedWrite(reg, mask, val) GPUCMD_AddSingleParam(GPUCMD_HEADER(0, (mask), (reg)), (val))
/// Adds a register write to the current command buffer.
#define GPUCMD_AddWrite(reg, val) GPUCMD_AddMaskedWrite((reg), 0xF, (val))
/// Adds multiple masked register writes to the current command buffer.
#define GPUCMD_AddMaskedWrites(reg, mask, vals, num) GPUCMD_Add(GPUCMD_HEADER(0, (mask), (reg)), (vals), (num))
/// Adds multiple register writes to the current command buffer.
#define GPUCMD_AddWrites(reg, vals, num) GPUCMD_AddMaskedWrites((reg), 0xF, (vals), (num))
/// Adds multiple masked incremental register writes to the current command buffer.
#define GPUCMD_AddMaskedIncrementalWrites(reg, mask, vals, num) GPUCMD_Add(GPUCMD_HEADER(1, (mask), (reg)), (vals), (num))
/// Adds multiple incremental register writes to the current command buffer.
#define GPUCMD_AddIncrementalWrites(reg, vals, num) GPUCMD_AddMaskedIncrementalWrites((reg), 0xF, (vals), (num))
