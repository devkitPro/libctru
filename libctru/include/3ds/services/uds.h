/**
 * @file uds.h
 * @brief UDS(NWMUDS) local-WLAN service. https://3dbrew.org/wiki/NWM_Services
 */
#pragma once

/// Node info struct.
typedef struct {
	u64 uds_friendcodeseed;//UDS version of the FriendCodeSeed.
	u8 usercfg[0x18];//This is the first 0x18-bytes from this config block: https://www.3dbrew.org/wiki/Config_Savegame#0x000A0000_Block
	u32 words_x20[2];//Not initialized by DLP-sysmodule.
} udsNodeInfo;

/**
 * @brief Initializes UDS.
 * @param sharedmem_size This must be 0x1000-byte aligned.
 * @param username Optional custom UTF-8 username(converted to UTF-16 internally) that other nodes on the UDS network can use. If not set the username from system-config is used. Max len is 10 characters without NUL-terminator.
 */
Result udsInit(u32 sharedmem_size, const uint8_t *username);

/// Exits UDS.
void udsExit(void);

/**
 * @brief Generates a NodeInfo struct with data loaded from system-config.
 * @param username If set, this is the UTF-8 string to convert for use in the struct. Max len is 10 characters without NUL-terminator.
 */
Result udsGenerateNodeInfo(udsNodeInfo *nodeinfo, const uint8_t *username);

/**
 * @brief Loads the UTF-16 username stored in the input NodeInfo struct, converted to UTF-8.
 * @param username This is the output UTF-8 string. Max len is 10 characters without NUL-terminator.
 */
Result udsGetNodeInfoUsername(udsNodeInfo *nodeinfo, uint8_t *username);

