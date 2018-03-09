/**
 * @file ns.h
 * @brief NS (Nintendo Shell) service.
 */
#pragma once

/// Initializes NS.
Result nsInit(void);

/// Exits NS.
void nsExit(void);

/**
 * @brief Launches a title and the required firmware (only if necessary).
 * @param titleid ID of the title to launch, 0 for gamecard, JPN System Settings' titleID for System Settings.
 */
Result NS_LaunchFIRM(u64 titleid);

/**
 * @brief Launches a title.
 * @param titleid ID of the title to launch, or 0 for gamecard.
 * @param launch_flags Flags used when launching the title.
 * @param procid Pointer to write the process ID of the launched title to.
 */
Result NS_LaunchTitle(u64 titleid, u32 launch_flags, u32 *procid);

/// Terminates the application from which this function is called
Result NS_TerminateTitle(void);
/**
 * @brief Launches a title and the required firmware.
 * @param titleid ID of the title to launch, 0 for gamecard.
 * @param flags Flags for firm-launch. bit0: require an application title-info structure in FIRM paramters to be specified via FIRM parameters. bit1: if clear, NS will check certain Configuration Memory fields.
 */
Result NS_LaunchApplicationFIRM(u64 titleid, u32 flags);

/**
 * @brief Reboots to a title.
 * @param mediatype Mediatype of the title.
 * @param titleid ID of the title to launch.
 */
Result NS_RebootToTitle(u8 mediatype, u64 titleid);

/**
 * @brief Terminates the process with the specified titleid.
 * @param titleid ID of the title to terminate.
 * @param timeout Timeout in nanoseconds. Pass 0 if not required.
 */
Result NS_TerminateProcessTID(u64 titleid, u64 timeout);

/// Reboots the system
Result NS_RebootSystem(void);