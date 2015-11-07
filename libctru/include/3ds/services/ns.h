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
 * @brief Launches a title.
 * @param titleid ID of the title to launch, or 0 for gamecard.
 * @param launch_flags Flags used when launching the title.
 * @param procid Pointer to write the process ID of the launched title to.
 */
Result NS_LaunchTitle(u64 titleid, u32 launch_flags, u32 *procid);

/**
 * @brief Reboots to a title.
 * @param mediatype Mediatype of the title.
 * @param titleid ID of the title to launch.
 */
Result NS_RebootToTitle(u8 mediatype, u64 titleid);
