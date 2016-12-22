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
Handle bossGetSessionHandle(void);

/**
 * @brief Register a task.
 * @param taskID BOSS taskID.
 * @param unk0 Unknown, usually zero.
 * @param unk1 Unknown, usually zero.
 */
Result bossRegisterTask(const char *taskID, u8 unk0, u8 unk1);

/**
 * @brief Send a property.
 * @param PropertyID PropertyID
 * @param buf Input buffer data.
 * @param size Buffer size.
 */
Result bossSendProperty(u16 PropertyID, const void* buf, u32 size);

/**
 * @brief ?
 * @param taskID BOSS taskID.
 */
Result bossStartTaskImmediate(const char *taskID);

/**
 * @brief ?
 * @param taskID BOSS taskID.
 */
Result bossStartBgImmediate(const char *taskID);

/**
 * @brief Deletes a task by using CancelTask and UnregisterTask internally.
 * @param taskID BOSS taskID.
 * @param unk Unknown, usually zero?
 */
Result bossDeleteTask(const char *taskID, u32 unk);

/**
 * @brief Returns task state.
 * @param taskID BOSS taskID.
 * @param inval Unknown, normally 0?
 * @param out0 Output field.
 * @param out1 Output field.
 * @param out2 Output field.
 */
Result bossGetTaskState(const char *taskID, s8 inval, u8 *out0, u32 *out1, u8 *out2);

/**
 * @brief This loads the current state of PropertyID 0x0 for the specified task.
 * @param taskID BOSS taskID.
 */
Result bossGetTaskProperty0(const char *taskID, u8 *out);

