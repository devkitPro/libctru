/**
 * @file pm.h
 * @brief PM (Process Manager) service.
 */
#pragma once

/// Initializes PM.
Result pmInit(void);

/// Exits PM.
void pmExit(void);

/**
 * @brief Launches a title.
 * @param mediatype Mediatype of the title.
 * @param titleid ID of the title.
 * @param launch_flags Flags to launch the title with.
 */
Result PM_LaunchTitle(u8 mediatype, u64 titleid, u32 launch_flags);

/**
 * @brief Gets launch flags from a title's exheader.
 * @param mediatype Mediatype of the title.
 * @param titleid ID of the title.
 * @param out Pointer to write the launch flags to.
 */
Result PM_GetTitleExheaderFlags(u8 mediatype, u64 titleid, u8* out);

/**
 * @brief Sets the current FIRM launch parameters.
 * @param size Size of the FIRM launch parameter buffer.
 * @param in Buffer to retrieve the launch parameters from.
 */
Result PM_SetFIRMLaunchParams(u32 size, u8* in);

/**
 * @brief Gets the current FIRM launch parameters.
 * @param size Size of the FIRM launch parameter buffer.
 * @param out Buffer to write the launch parameters to.
 */
Result PM_GetFIRMLaunchParams(u32 size, u8* out);

/**
 * @brief Sets the current FIRM launch parameters.
 * @param firm_titleid_low Low Title ID of the FIRM title to launch.
 * @param size Size of the FIRM launch parameter buffer.
 * @param in Buffer to retrieve the launch parameters from.
 */
Result PM_LaunchFIRMSetParams(u32 firm_titleid_low, u32 size, u8* in);

/**
 * @brief Terminates the current Application
 * @param timeout Timeout in nanoseconds
 */
Result PM_TerminateCurrentApplication(u64 timeout);

/**
 * @brief Terminates the specified Process
 * @param pid Process-ID of the process to terminate
 * @param timeout Timeout on nanoseconds
 */
Result PM_TerminateProcess(u8 pid, u64 timeout);

/**
 * @brief Unregisters a process
 * @param tid TitleID of the process to unregister
 */
 Result PM_UnregisterProcess(u64 tid);