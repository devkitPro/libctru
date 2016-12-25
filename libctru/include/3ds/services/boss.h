/**
 * @file boss.h
 * @brief BOSS service, see also: https://www.3dbrew.org/wiki/BOSS_Services
 */
#pragma once

/// BOSS context.
typedef struct
{
	u32 property[0x7];

	char url[0x200];

	u32 property_x8;
	u8 property_x9;

	u8 property_xa[0x100];

	u8 property_xb[0x200];

	char property_xd[0x360];//Additonal optional HTTP request headers.

	u32 property_xe;

	u32 property_xf[0xc>>2];

	u8 property_x10;
	u8 property_x11;
	u8 property_x12;
	u32 property_x13;
	u32 property_x14;

	u8 property_x15[0x40];

	u32 property_x16;

	u32 property_x3b;

	u8 property_x3e[0x200];
} bossContext;

typedef enum {
	BOSSTASKSTATUS_STARTED = 0x2,
	BOSSTASKSTATUS_ERROR = 0x7,
} bossTaskStatus;

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
 * @param status Output status, see bossTaskStatus.
 * @param out1 Output field.
 * @param out2 Output field.
 */
Result bossGetTaskState(const char *taskID, s8 inval, u8 *status, u32 *out1, u8 *out2);

/**
 * @brief This loads the current state of PropertyID 0x0 for the specified task.
 * @param taskID BOSS taskID.
 */
Result bossGetTaskProperty0(const char *taskID, u8 *out);

/**
 * @brief Setup a BOSS context with the default config.
 * @param bossContext BOSS context.
 * @param seconds_interval Interval in seconds for running the task automatically.
 * @param url Task URL.
 */
void bossSetupContextDefault(bossContext *ctx, u32 seconds_interval, const char *url);

/**
 * @brief Sends the config stored in the context. Used before registering a task.
 * @param bossContext BOSS context.
 */
Result bossSendContextConfig(bossContext *ctx);

