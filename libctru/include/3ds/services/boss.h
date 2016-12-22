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
 * @brief Register a task.
 * @param taskID BOSS taskID.
 * @param unk0 Unknown, usually zero.
 * @param unk1 Unknown, usually zero.
 */
Result bossRegisterTask(char *taskID, u8 unk0, u8 unk1);

/**
 * @brief Send a property.
 * @param PropertyID PropertyID
 * @param buf Input buffer data.
 * @param size Buffer size.
 */
Result bossSendProperty(u16 PropertyID, void* buf, u32 size);

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

/**
 * @brief Deletes a task by using CancelTask and UnregisterTask internally.
 * @param taskID BOSS taskID.
 * @param unk Unknown, usually zero?
 */
Result bossDeleteTask(char *taskID, u32 unk);

/**
 * @brief ?
 * @param taskID BOSS taskID.
 */
Result bossCmd34(char *taskID);

