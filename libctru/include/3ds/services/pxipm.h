/**
 * @file pxipm.h
 * @brief Process Manager PXI service
 */

#pragma once

#include <3ds/exheader.h>
#include <3ds/services/fs.h>

/// Initializes PxiPM.
Result pxiPmInit(void);

/// Exits PxiPM.
void pxiPmExit(void);

/**
 * @brief Gets the current PxiPM session handle.
 * @return The current PxiPM session handle.
 */
Handle *pxiPmGetSessionHandle(void);

/**
 * @brief Retrives the exheader information set(s) (SCI+ACI) about a program.
 * @param exheaderInfos[out] Pointer to the output exheader information set.
 * @param programHandle The program handle.
 */
Result PXIPM_GetProgramInfo(ExHeader_Info *exheaderInfo, u64 programHandle);

/**
 * @brief Loads a program and registers it to Process9.
 * @param programHandle[out] Pointer to the output the program handle to.
 * @param programInfo Information about the program to load.
 * @param updateInfo Information about the program update to load.
 */
Result PXIPM_RegisterProgram(u64 *programHandle, const FS_ProgramInfo *programInfo, const FS_ProgramInfo *updateInfo);

/**
 * @brief Unloads a program and unregisters it from Process9.
 * @param programHandle The program handle.
 */
Result PXIPM_UnregisterProgram(u64 programHandle);
