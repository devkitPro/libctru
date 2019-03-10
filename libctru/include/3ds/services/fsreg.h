/**
 * @file fsReg.h
 * @brief Filesystem registry service
 */

#pragma once

#include <3ds/exheader.h>
#include <3ds/services/fs.h>

/// Initializes fs:REG.
Result fsRegInit(void);

/// Exits fs:REG.
void fsRegExit(void);

/**
 * @brief Gets the current fs:REG session handle.
 * @return The current fs:REG session handle.
 */
Handle *fsRegGetSessionHandle(void);

/**
 * @brief Registers a program's storage information.
 * @param pid The Process ID of the program.
 * @param programHandle The program handle.
 * @param programInfo Information about the program.
 * @param storageInfo Storage information to register.
 */
Result FSREG_Register(u32 pid, u64 programHandle, const FS_ProgramInfo *programInfo, const ExHeader_Arm11StorageInfo *storageInfo);

/**
 * @brief Unregisters a program's storage information.
 * @param pid The Process ID of the program.
 */
Result FSREG_Unregister(u32 pid);

/**
 * @brief Retrives the exheader information set(s) (SCI+ACI) about a program.
 * @param exheaderInfos[out] Pointer to the output exheader information set(s).
 * @param maxNumEntries The maximum number of entries.
 * @param programHandle The program handle.
 */
Result FSREG_GetProgramInfo(ExHeader_Info *exheaderInfos, u32 maxNumEntries, u64 programHandle);

/**
 * @brief Loads a program.
 * @param programHandle[out] Pointer to the output the program handle to.
 * @param programInfo Information about the program to load.
 */
Result FSREG_LoadProgram(u64 *programHandle, const FS_ProgramInfo *programInfo);

/**
 * @brief Unloads a program.
 * @param programHandle The program handle.
 */
Result FSREG_UnloadProgram(u64 programHandle);

/**
 * @brief Checks if a program has been loaded by fs:REG.
 * @param programHandle The program handle.
 */
Result FSREG_CheckHostLoadId(u64 programHandle);
