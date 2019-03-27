/**
 * @file pmdbg.h
 * @brief PM (Process Manager) debug service.
 */
#pragma once

#include <3ds/services/pmapp.h>

/// Initializes pm:dbg.
Result pmDbgInit(void);

/// Exits pm:dbg.
void pmDbgExit(void);

/**
 * @brief Gets the current pm:dbg session handle.
 * @return The current pm:dbg session handle.
 */
Handle *pmDbgGetSessionHandle(void);

/**
 * @brief Enqueues an application for debug after setting cpuTime to 0, and returns a debug handle to it.
 * If another process was enqueued, this just calls @ref RunQueuedProcess instead.
 * @param[out] Pointer to output the debug handle to.
 * @param programInfo Program information of the title.
 * @param launchFlags Flags to launch the title with.
 */
Result PMDBG_LaunchAppDebug(Handle *outDebug, const FS_ProgramInfo *programInfo, u32 launchFlags);

/**
 * @brief Launches an application for debug after setting cpuTime to 0.
 * @param programInfo Program information of the title.
 * @param launchFlags Flags to launch the title with.
 */
Result PMDBG_LaunchApp(const FS_ProgramInfo *programInfo, u32 launchFlags);

/**
 * @brief Runs the queued process and returns a debug handle to it.
 * @param[out] Pointer to output the debug handle to.
 */
Result PMDBG_RunQueuedProcess(Handle *outDebug);
