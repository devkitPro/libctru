/**
 * @file cecd.h
 * @brief CECD service, see also: https://www.3dbrew.org/wiki/CECD_Services
 */
#pragma once

#include <3ds/types.h>

typedef enum {
	CEC_PATH_MBOX_LIST = 1,
	CEC_PATH_MBOX_INFO = 2,
	CEC_PATH_INBOX_INFO = 3,
	CEC_PATH_OUTBOX_INFO = 4,
	CEC_PATH_OUTBOX_INDEX = 5,
	CEC_PATH_INBOX_MSG = 6,
	CEC_PATH_OUTBOX_MSG = 7,
	CEC_PATH_ROOT_DIR = 10,
	CEC_PATH_MBOX_DIR = 11,
	CEC_PATH_INBOX_DIR = 12,
	CEC_PATH_OUTBOX_DIR = 13,
} CecPath;

#define CECMESSAGE_BOX_ICON 101
#define CECMESSAGE_BOX_TITLE 110

typedef enum {
	CEC_COMMAND_NONE = 0,
	CEC_COMMAND_START = 1,
	CEC_COMMAND_RESET_START = 2,
	CEC_COMMAND_READYSCAN = 3,
	CEC_COMMAND_READYSCANWAIT = 4,
	CEC_COMMAND_STARTSCAN = 5,
	CEC_COMMAND_RESCAN = 6,
	CEC_COMMAND_NDM_RESUME = 7,
	CEC_COMMAND_NDM_SUSPEND = 8,
	CEC_COMMAND_NDM_SUSPEND_IMMEDIATE = 9,
	CEC_COMMAND_STOPWAIT = 0xA,
	CEC_COMMAND_STOP = 0xB,
	CEC_COMMAND_STOP_FORCE = 0xC,
	CEC_COMMAND_STOP_FORCE_WAIT = 0xD,
	CEC_COMMAND_RESET_FILTER = 0xE,
	CEC_COMMAND_DAEMON_STOP = 0xF,
	CEC_COMMAND_DAEMON_START = 0x10,
	CEC_COMMAND_EXIT = 0x11,
	CEC_COMMAND_OVER_BOSS = 0x12,
	CEC_COMMAND_OVER_BOSS_FORCE = 0x13,
	CEC_COMMAND_OVER_BOSS_FORCE_WAIT = 0x14,
	CEC_COMMAND_END = 0x15,
} CecCommand;

typedef enum {
	CEC_STATE_ABBREV_IDLE = 1,
	CEC_STATE_ABBREV_INACTIVE = 2,
	CEC_STATE_ABBREV_SCANNING = 3,
	CEC_STATE_ABBREV_WLREADY = 4,
	CEC_STATE_ABBREV_OTHER = 5,
} CecStateAbbrev;

typedef enum {
	CEC_EXT_HEADER_TYPE_ICON = 2,
	CEC_EXT_HEADER_TYPE_GAME_NAME = 3,
	CEC_EXT_HEADER_TYPE_NOTIFICATION_TEXT = 4,
	CEC_EXT_HEADER_TYPE_REGION = 5,
} CecExtHeaderType;

typedef u8 CecMessageId[8];

typedef struct SlotMetadata {
	int send_method;
	u32 title_id;
	u32 size;
} CecSlotMetadata;

/**
 * @brief Initializes CECD.
 * @param force_user When true, just use cecdU instead of trying to initialize with cecdS first.
 */
Result cecdInit(bool force_user);

/**
 * @brief Reads a CEC message
 * @param title_id The title id of the message we want to fetch
 * @param is_outbox Are we fetching from an outbox?
 * @param message_id The message id of the message to fetch
 * @param buf the output buffer
 * @param size the size of the output buffer
 */
Result cecdReadMessage(u32 title_id, bool is_outbox, CecMessageId message_id, u8* buf, u32 size);

/**
 * @brief Reads a CEC message with hmac
 * @param title_id The title id of the message we want to fetch
 * @param is_outbox Are we fetching from an outbox?
 * @param message_id The message id of the message to fetch
 * @param buf the output buffer
 * @param size the size of the output buffer
 * @param hmac the hmac of the message
 */
Result cecdReadMessageWithHMAC(u32 title_id, bool is_outbox, CecMessageId message_id, u8* buf, u32 size, u8* hmac);

/**
 * @brief Writes a CEC message
 * @param title_id The title id of the message we want to write
 * @param is_outbox Are we fetching from an outbox?
 * @param message_id The message id of the message to write
 * @param buf the input buffer
 * @param size the size of the input buffer
 */
Result cecdWriteMessage(u32 title_id, bool is_outbox, CecMessageId message_id, u8* buf, u32 size);

/**
 * @brief Writes a CEC message with hmac
 * @param title_id The title id of the message we want to write
 * @param is_outbox Are we fetching from an outbox?
 * @param message_id The message id of the message to write
 * @param buf the input buffer
 * @param size the size of the input buffer
 * @param hmac the hmac of the message
 */
Result cecdWriteMessageWithHMAC(u32 title_id, bool is_outbox, CecMessageId message_id, u8* buf, u32 size, u8* hmac);

/**
 * @brief Executes start for a given command
 * @param command Command to start
 */
Result cecdStart(CecCommand command);

/**
 * @brief Executes stop for a given command
 * @param command Command to stop
 */
Result cecdStop(CecCommand command);

/**
 * @brief Get the state of CECD
 * @param state The output state
 */
Result cecdGetState(CecStateAbbrev* state);

/**
 * @brief Get the info event handle
 * @param handle The handle
 */
Result cecdGetCecInfoEventHandle(Handle* handle);

/**
 * @brief Gets the change state event handle
 * @param handle The handle
 */
Result cecdGetChangeStateEventHandle(Handle* handle);

/**
 * @brief Opens and writes to a CEC data file
 * @param title_id The title id to manipulate
 * @param path_type The path of what is being written to
 * @param buf The write buffer
 * @param size The size of the write buffer
 */
Result cecdOpenAndWrite(u32 title_id, CecPath path_type, u8* buf, u32 size);

/**
 * @brief Opens and reads from a CEC data file
 * @param title_id The title id to manipulate
 * @param path_type The path of what is being read from
 * @param buf The read buffer
 * @param size The size of the read buffer
 */
Result cecdOpenAndRead(u32 title_id, CecPath path_type, u8* buf, u32 size);

/**
 * @brief Creates an SPR request
 */
Result cecdSprCreate(void);

/**
 * @brief Initialises an SPR request
 */
Result cecdSprInitialise(void);

/**
 * @brief Get the metadata for all slots to send
 * @param buf The destination buffer of the slot metadata
 * @param size The size of the destination buffer
 * @param slots_total Outputs the total amount of slots being written 
 */
Result cecdSprGetSlotsMetadata(CecSlotMetadata* buf, u32 size, u32* slots_total);

/**
 * @brief Get the slot to send
 * @param title_id The title id of the slot to get
 * @param buf The read buffer
 * qparam size The size of the read buffer
 */
Result cecdSprGetSlot(u32 title_id, u8* buf, u32 size);

/**
 * @brief Set if a title was sent
 * @param title_id The title id we want to set as sent
 * @param success If the title id was sent successfully
 */
Result cecdSprSetTitleSent(u32 title_id, bool success);

/**
 * @brief Finalise sending slots in an SPR request
 */
Result cecdSprFinaliseSend(void);

/**
 * @brief Start receiving remote data of an SPR request
 */
Result cecdSprStartRecv(void);

/**
 * @brief Adds the slots metadata you received
 * @param buf The buffer of the slots metadata
 * @param size The size of the buffer
 */
Result cecdSprAddSlotsMetadata(u8* buf, u32 size);

/**
 * @brief Add a received slot
 * @param title_id The title_id of the slot to add
 * @param buf The buffer of the slot to add
 * @param size The size of the slot to add
 */
Result cecdSprAddSlot(u32 title_id, u8* buf, u32 size);

/**
 * @brief Finalise receiving the SPR request
 */
Result cecdSprFinaliseRecv(void);

/**
 * @brief Signals that the SPR request is done
 * @param success Set if the request was successful
 */
Result cecdSprDone(bool success);

/**
 * @brief Gets the boss user ID needed to send SPR requests
 * @param out The output of the boss user id
 */
Result cecdGetBossUserid(u64* out);

/**
 * @brief Gets the open CECD handle
 */
Handle cecdGetServHandle(void);
