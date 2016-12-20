/**
 * @file boss.h
 * @brief BOSS service, see also: https://www.3dbrew.org/wiki/BOSS_Services
 */
#pragma once

/**
 * @brief Initializes BOSS.
 * @param programID programID to use, 0 for the current process. Not used internally unless BOSSP is available.
 */
Result bossInit(u64 programID);

/// Exits BOSS.
void bossExit(void);

/// Returns the BOSS session handle.
Handle bossGetSessionHandle();

/**
 * @brief ?
 * @param taskID BOSS taskID.
 */
Result bossStartTaskImmediate(char *taskID);

/**
 * @brief ?
 * @param taskID BOSS taskID.
 */
Result bossStartBgImmediate(char *taskID);

