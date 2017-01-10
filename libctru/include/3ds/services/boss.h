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

/// BOSS task status.
typedef enum {
	BOSSTASKSTATUS_STARTED = 0x2,
	BOSSTASKSTATUS_ERROR = 0x7,
} bossTaskStatus;

/// Type values for bossGetNsDataHeaderInfo().
typedef enum {
	bossNsDataHeaderInfoType_ContentSize = 0x3 /// Size of the content.
} bossNsDataHeaderInfoTypes;

/// Size of the output data for bossGetNsDataHeaderInfo().
typedef enum {
	bossNsDataHeaderInfoTypeSize_ContentSize = 0x4 ///Type2
} bossNsDataHeaderInfoTypeSizes;

/**
 * @brief Initializes BOSS.
 * @param programID programID to use, 0 for the current process. Only used when BOSSP is available without *hax payload.
 * @param force_user When true, just use bossU instead of trying to initialize with bossP first.
 */
Result bossInit(u64 programID, bool force_user);

/**
 * @brief Run the InitializeSession service cmd. This is mainly for changing the programID associated with the current BOSS session.
 * @param programID programID to use, 0 for the current process.
 */
Result bossReinit(u64 programID);

/// Exits BOSS.
void bossExit(void);

/// Returns the BOSS session handle.
Handle bossGetSessionHandle(void);

/**
 * @brief Set the content data storage location.
 * @param extdataID u64 extdataID, must have the high word set to the shared-extdata value when it's for NAND.
 * @param boss_size Probably the max size in the extdata which BOSS can use.
 * @param mediaType Roughly the same as FS mediatype.
 */
Result bossSetStorageInfo(u64 extdataID, u32 boss_size, u8 mediaType);

/**
 * @brief Unregister the content data storage location, which includes unregistering the BOSS-session programID with BOSS.
 */
Result bossUnregisterStorage(void);

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
 * @brief Deletes the content file for the specified NsDataId.
 * @param NsDataId NsDataId
 */
Result bossDeleteNsData(u32 NsDataId);

/**
 * @brief Gets header info for the specified NsDataId.
 * @param NsDataId NsDataId
 * @param type Type of data to load.
 * @param buffer Output buffer.
 * @param size Output buffer size.
 */
Result bossGetNsDataHeaderInfo(u32 NsDataId, u8 type, void* buffer, u32 size);

/**
 * @brief Reads data from the content for the specified NsDataId.
 * @param NsDataId NsDataId
 * @param offset Offset in the content.
 * @param buffer Output buffer.
 * @param size Output buffer size.
 * @param transfer_total Optional output actual read size, can be NULL.
 * @param unk_out Optional unknown output, can be NULL.
 */
Result bossReadNsData(u32 NsDataId, u64 offset, void* buffer, u32 size, u32 *transfer_total, u32 *unk_out);

/**
 * @brief Starts a task soon after running this command.
 * @param taskID BOSS taskID.
 */
Result bossStartTaskImmediate(const char *taskID);

/**
 * @brief Similar to bossStartTaskImmediate?
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

