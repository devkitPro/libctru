/**
 * @file loader.h
 * @brief LOADER Service
 */

#pragma once

#include <3ds/exheader.h>
#include <3ds/services/fs.h>

/// Initializes LOADER.
Result loaderInit(void);

/// Exits LOADER.
void loaderExit(void);

/**
 * @brief Loads a program and returns a process handle to the newly created process.
 * @param[out] process Pointer to output the process handle to.
 * @param programHandle The handle of the program to load.
 */
Result LOADER_LoadProcess(Handle* process, u64 programHandle);

/**
 * @brief Registers a program (along with its update).
 * @param[out] programHandle Pointer to output the program handle to.
 * @param programInfo The program info.
 * @param programInfo The program update info.
 */
Result LOADER_RegisterProgram(u64* programHandle, const FS_ProgramInfo *programInfo, const FS_ProgramInfo *programInfoUpdate);

/**
 * @brief Unregisters a program (along with its update).
 * @param programHandle The handle of the program to unregister.
 */
Result LOADER_UnregisterProgram(u64 programHandle);

/**
 * @brief Retrives a program's main NCCH extended header info (SCI + ACI, see @ref ExHeader_Info).
 * @param[out] exheaderInfo Pointer to output the main NCCH extended header info.
 * @param programHandle The handle of the program to unregister
 */
Result LOADER_GetProgramInfo(ExHeader_Info* exheaderInfo, u64 programHandle);
