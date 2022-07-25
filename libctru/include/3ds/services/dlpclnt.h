/**
 * @file dlpclnt.h
 * @brief dlp::clnt (download play client) service.
 */
#pragma once

 /// Download play client state.
typedef enum {
   DLPCLNT_STATE_IDLE = 1,
   DLPCLNT_STATE_SCANNING = 2,
   DLPCLNT_STATE_JOINED = 5,
   DLPCLNT_STATE_DOWNLOADING = 6,
   DLPCLNT_STATE_COMPLETE = 9
} dlpClntState;

/// Info about a scanned title
typedef struct {
   u32 uniqueId;
   u32 variation;
   u8 macAddr[6];
   u16 version; // XX: probably?
   u8 ageRatings[16];
   u16 shortDescription[64]; // UTF-16
   u16 longDescription[128]; // UTF-16
   u8 icon[0x1200];  // 48x48, RGB565
   u32 size;
   u8 unknown2;
   u8 unknown3;
   u16 padding;
} dlpClntTitleInfo;

/// Information about dlp client's status.
typedef struct {
   dlpClntState state;
   u32 unitsTotal;
   u32 unitsRecvd;
} dlpClntMyStatus;

/// Initializes DLP client.
Result dlpClntInit(void);

/// Exits DLP client.
void dlpClntExit(void);

/**
 * @brief Waits for the dlp event to occur, or checks if the event was signaled.
 * @return Always true. However if wait=false, this will return false if the event wasn't signaled.
 * @param nextEvent Whether to discard the current event and wait for the next event.
 * @param wait When true this will not return until the event is signaled. When false this checks if the event was signaled without waiting for it.
 */
bool dlpClntWaitForEvent(bool nextEvent, bool wait);

/**
* @brief Calculates the aligned shared memory size to use with dlp.
* @return The calculated aligned memory size to use.
* @param maxTitles Maximum amount of titles that can be found in a scan at once. Cannot be larger than 16.
* @param constantMemSize Must be between 0x100000 and 0x200000.
*/
size_t dlpCalcSharedMemSize(u8 maxTitles, size_t constantMemSize);

/**
* @brief Forms the title id of a title's dlp child.
* @return The dlp child title id.
* @param uniqueId The title's unique id.
* @param variation The title's variation.
*/
u64 dlpCreateChildTid(u32 uniqueId, u32 variation);

/**
* @brief Initializes dlp clnt.
* @param sharedMemSize Size of the shared memory.
* @param maxScanTitles Maximum amount of titles that can be found in a scan at once. Cannot be larger than 16.
* @param constantMemSize Must be between 0x100000 and 0x200000.
* @param sharedMemHandle Shared memory handle.
* @param eventHandle Event handle that will be signaled by dlp clnt.
*/
Result DLPCLNT_Initialize(size_t sharedMemSize, u8 maxScanTitles, size_t constantMemSize, Handle sharedmemHandle, Handle eventHandle);

/// Finalizes dlp clnt.
Result DLPCLNT_Finalize(void);

/**
* @brief Gets channel.
* @paramt channel Pointer to output channel to.
*/
Result DLPCLNT_GetChannel(u16* channel);

/**
* @brief Begin scanning for dlp servers.
* @param channel Channel to use.
* @param macAddr Optional mac address to filter detected dlp servers. Must be 6 bytes.
* @param tidFilter If not 0, filters detected dlp child titles to specified title id.
*/
Result DLPCLNT_StartScan(u16 channel, u8* macAddrFilter, u64 tidFilter);

/// Stop scanning for dlp servers.
Result DLPCLNT_StopScan(void);

/**
* @brief Get title info from scan.
* @param titleInfo Pointer to write title info to.
* @param actual_size Optional pointer to output actual title size written.
* @param macAddr Mac address of server. Must be 6 bytes.
* @param uniqueId Unique id of title.
* @param variation Variation of title.
*/
Result DLPCLNT_GetTitleInfo(dlpClntTitleInfo* titleInfo, size_t* actual_size, u8* macAddr, u32 uniqueId, u32 variation);

/**
* @brief Get available title info from scan, getting the next available title info on the next call.
* @param titleInfo Pointer to write title info to.
* @param actual_size Optional pointer to output actual title size written to buffer.
*/
Result DLPCLNT_GetTitleInfoInOrder(dlpClntTitleInfo* titleInfo, size_t* actual_size);

/**
* @brief Prepares for system download for system update.
* @param macAddr Mac address of server to download from. Must be 6 bytes.
* @param uniqueId Unique id of title advertised by server.
* @param variation Variation of title advertised by server.
*/
Result DLPCLNT_PrepareForSystemDownload(u8* macAddr, u32 uniqueId, u32 variation);

/// Joins dlp session and waits for server to begin distributing system update.
Result DLPCLNT_StartSystemDownload(void);

/**
* @brief Joins dlp session and waits for server to begin distributing dlp child.
* @param macAddr Mac address of server to join and download from. Must be 6 bytes.
* @param uniqueId Unique id of title advertised by server.
* @param variation Variation of title advertised by server.
*/
Result DLPCLNT_StartTitleDownload(u8* macAddr, u32 uniqueId, u32 variation);

/**
* @brief Gets dlp status information.
* @param status Status pointer to output to.
*/
Result DLPCLNT_GetMyStatus(dlpClntMyStatus* status);

/**
* @brief Gets dlp wireless reboot passphrase.
* @param buf Buffer to write reboot passphrase to. Must be 9 bytes.
*/
Result DLPCLNT_GetWirelessRebootPassphrase(void* buf);

/// Disconnects from dlp server.
Result DLPCLNT_StopSession(void);
