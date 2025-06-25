/**
 * @file am.h
 * @brief AM (Application Manager) services.
 */
#pragma once

#include <3ds/services/fs.h>

/// Contains basic information about a title.
typedef struct
{
	u64 titleID;   ///< The title's ID.
	u64 size;      ///< The title's installed size.
	u16 version;   ///< The title's version.
	u32 titleType; ///< The title's type from the TMD.
} AM_TitleInfo;

/// Contains basic information about a title.
typedef AM_TitleInfo AM_TitleEntry;

/// Contains information about a ticket.
typedef struct
{
	u64 titleId; ///< Title ID of the corresponding title.
	u64 ticketId; ///< Ticket ID.
	u16 titleVersion; ///< Title version at the time the ticket was issued.
	u32 size; ///< Total size of the ticket.
} AM_TicketInfo;

/// Ticket limit flags for ticket limit infos.
typedef enum
{
	TICKET_LIMIT_PLAY_COUNT = BIT(3) ///< Ticket limit for a maximum play count (used for demos).
} AM_TicketLimitFlag;

/// Contains information about the limits section of a ticket.
typedef struct
{
	u8 flags;        ///< Enabled ticket limit types. @ref AM_TicketLimitFlag
	u8 maxPlaycount; ///< The maximum play count when the ticket has a play count limit.
	u8 reserved[14];
} AM_TicketLimitInfo;

/// Demo limit information.
typedef AM_TicketLimitInfo AM_DemoLaunchInfo;

/// Pending title status values.
typedef enum
{
	AM_INSTALL_NONE               = 0, ///< No installation in progress.
	AM_INSTALL_WAITING_FOR_IMPORT = 1, ///< Installation is pending.
	AM_INSTALL_RESUMABLE          = 2, ///< Installation is paused.
	AM_INSTALL_WAITING_FOR_COMMIT = 3, ///< Installation is awaiting committing to the title database.
	AM_INSTALL_ALREADY_EXISTS     = 4, ///< The title already exists.
	AM_INSTALL_DELETING           = 5, ///< The title is pending removal.
	AM_INSTALL_NEEDS_CLEANUP      = 6, ///< Installation requires cleanup.
} AM_TitleInstallStatus;

/// Content import status values.
typedef enum
{
	AM_CONTENT_IN_PROGRESS    = 1, ///< Content installation is in progress.
	AM_CONTENT_PAUSED         = 2, ///< Content installation is paused.
	AM_CONTENT_FINISHED       = 3, ///< Content installation is complete.
	AM_CONTENT_ALREADY_EXISTS = 4, ///< The content already exists.
} AM_ContentInstallStatus;

// Contains basic information about a pending title.
typedef struct
{
	u64 titleId;   ///< The title's title ID.
	u16 version;   ///< The title's title version.
	u8 status;     ///< The title's installation status. @ref AM_TitleInstallStatus
	u32 titleType; ///< The title's title type from the TMD.
	u64 titleSize; ///< The amount of storage space (in bytes) the title takes up.
} AM_PendingTitleInfo;

// Contains basic information about a pending title.
typedef AM_PendingTitleInfo AM_PendingTitleEntry;

// Contains information about a title's active content import.
typedef struct
{
	u32 contentId;            ///< Content ID (from the TMD) of this content.
	u16 contentIndex;         ///< Content index (from the TMD) of this content.
	u8 status;                ///< Installation status of this content. @ref AM_ContentInstallStatus
	u64 size;                 ///< Total size of this content.
	u64 currentInstallOffset; ///< Current installation offset for this content.
} AM_ImportContentContext;

/// Pending title flags.
typedef enum
{
	AM_PENDING_NON_SYSTEM = BIT(0), ///< Non-system titles.
	AM_PENDING_SYSTEM     = BIT(1)  ///< System titles.
} AM_PendingTitleFlags;

/// Information about an exported DSiWare title.
typedef struct AM_TWLBackupInfo
{
	u64 titleId;         ///< Title ID of the exported title.
	u16 groupId;         ///< Group ID (from the TMD).
	u16 titleVersion;    ///< Title version of the exported title.
	u32 publicSaveSize;  ///< public.sav size.
	u32 privateSaveSize; ///< private.sav size.
	u64 required_size;   ///< Total size (in bytes) required to install the exported title.
} AM_TWLBackupInfo;

/// Information about the TWL NAND partition.
typedef struct {
	u64 capacity;        ///< Total capacity.
	u64 freeSpace;       ///< Total free space.
	u64 titlesCapacity;  ///< Capacity for titles.
	u64 titlesFreeSpace; ///< Free space for titles.
} AM_TWLPartitionInfo;

/// Title ContentInfo flags.
typedef enum
{
	AM_CONTENT_INSTALLED = BIT(0), ///< The content is installed.
	AM_CONTENT_OWNED     = BIT(1)  ///< The user has the right to use the content according to one or more of the title's tickets.
} AM_ContentInfoFlags;

/// Title Content flags.
typedef enum
{
	AM_CONTENT_ENCRYPTED = BIT(0), ///< Content is encrypted (CDN crypto).
	AM_CONTENT_DISC      = BIT(1),
	AM_CONTENT_HASHED    = BIT(2),
	AM_CONTENT_CFM       = BIT(3),
	AM_CONTENT_SHA1_HASH = BIT(13),
	AM_CONTENT_OPTIONAL  = BIT(14), ///< Content is optional (set for non-contentindex-0 DLC contents)
	AM_CONTENT_SHARED    = BIT(15)
} AM_ContentTypeFlags;

/// Contains information about a title's content.
typedef struct
{
	u16 index;     ///< Content index from the TMD.
	u16 type;      ///< Content flags from the TMD. @ref AM_ContentTypeFlags
	u32 contentId; ///< Content ID from the TMD.
	u64 size;      ///< Size of the content in the title.
	u8 flags;      ///< @ref AM_ContentInfoFlags
	u8 padding[7]; ///< Padding
} AM_ContentInfo;

/// Contains internal information about a title's storage location.
typedef struct
{
	u32 cmdFileId;        ///< The ID of the currently active content metadata (CMD) file.
	u32 saveFileId;       ///< The ID of the currently active save data file.
	u32 cmdFormatVersion; ///< The format version of the currently active content metadata (CMD) file.
	u32 reserved[5];
} AM_InternalTitleLocationInfo;

/// Initializes AM. This doesn't initialize with "am:app", see amAppInit().
Result amInit(void);

/// Initializes AM with a service which has access to the AMAPP-commands. This should only be used when using the AMAPP commands, as non-AMAPP commands will be inaccessible in this case.
Result amAppInit(void);

/// Exits AM.
void amExit(void);

/// Gets the current AM session handle.
Handle *amGetSessionHandle(void);

/**
 * @brief Gets the number of titles for a given media type.
 * @param mediatype Media type to get titles from.
 * @param[out] count Pointer to write the title count to.
 */
Result AM_GetTitleCount(FS_MediaType mediatype, u32 *count);

/**
 * @brief Gets a list of title IDs present in a mediatype.
 * @param[out] titlesRead Pointer to output the number of read titles to.
 * @param mediatype Media type to get titles from.
 * @param titleCount Number of title IDs to get.
 * @param[out] titleIds Buffer to output the retrieved title IDs to.
 */
Result AM_GetTitleList(u32* titlesRead, FS_MediaType mediatype, u32 titleCount, u64 *titleIds);

/**
 * @brief Gets a list of details about installed titles.
 * @param mediatype Media type to get titles from.
 * @param titleCount Number of titles to list.
 * @param titleIds List of title IDs to retrieve details for.
 * @param[out] titleInfo Buffer to write AM_TitleInfo's to.
 */
Result AM_GetTitleInfo(FS_MediaType mediatype, u32 titleCount, u64 *titleIds, AM_TitleInfo *titleInfo);

/**
 * @brief Deletes a title, provided that it is not a system title.
 * @param mediatype Media type to delete from.
 * @param titleID ID of the title to delete.
 */
Result AM_DeleteUserTitle(FS_MediaType mediatype, u64 titleID);

/**
 * @brief Gets the product code of a title.
 * @param mediatype Media type of the title.
 * @param titleID ID of the title.
 * @param[out] productCode Pointer to output the product code to. (length = 16)
 */
Result AM_GetTitleProductCode(FS_MediaType mediatype, u64 titleId, char *productCode);

/**
 * @brief Gets the ext data ID of a title.
 * @param[out] extDataId Pointer to output the ext data ID to.
 * @param mediatype Media type of the title.
 * @param titleID ID of the title.
 */
Result AM_GetTitleExtDataId(u64 *extDataId, FS_MediaType mediatype, u64 titleId);

/**
 * @brief Deletes all tickets for a specific title.
 * @param titleId ID of the ticket to delete.
 */
Result AM_DeleteTicket(u64 titleId);

/**
 * @brief Gets the number of tickets installed on the system.
 * @param[out] count Pointer to output the ticket count to.
 */
Result AM_GetTicketCount(u32 *count);

/**
 * @brief Gets a list of title IDs for which tickets are installed.
 * @param[out] ticketsRead Pointer to output the number of read ticket title IDs to.
 * @param ticketCount Number of ticket title IDs to read.
 * @param skip Offset to start from in the entire ticket title ID list.
 * @param[out] ticketTitleIds Buffer to output the retrieved ticket title IDs to.
 */
Result AM_GetTicketList(u32 *ticketsRead, u32 ticketCount, u32 skip, u64 *ticketTitleIds);

/**
 * @brief Gets a 32-bit device-specific ID.
 * @param[out] outInternalResult Pointer to write an internal pxi:am9 result value to.
 * @param[out] deviceID Pointer to write the device ID to.
 */
Result AM_GetDeviceId(s32 *outInternalResult, u32 *outDeviceId);

/**
 * @brief Returns the number of active title installation contexts (including system and user titles).
 * @param[out] outNum Pointer to output the number of active title installation contexts to.
 * @param mediaType Media type to check for title installation contexts.
 */
Result AM_GetNumPendingTitles(u32 *outNum, FS_MediaType mediaType);

/**
 * @brief Returns the title IDs of active title installation contexts (including system and user titles).
 * @param[out] outNum Pointer to output the number of retrived title IDs to.
 * @param[out] outTitleIds Pointer to output the retrieved title IDs to.
 * @param count The maximum number of title IDs to retrieve.
 * @param mediaType The media type to use for retrieving the title IDs for active title import contexts.
 */
Result AM_GetPendingTitleList(u32 *outNum, u64 *outTitleIds, u32 count, FS_MediaType mediaType);

/**
 * @brief Gets information about pending titles on this system.
 * @param titleCount Number of pending titles to read.
 * @param mediatype Media type of pending titles to get information on.
 * @param titleIds IDs of the titles to get information about.
 * @param[out] titleInfo Buffer to output the retrieved pending title infos to.
 */
Result AM_GetPendingTitleInfo(u32 titleCount, FS_MediaType mediatype, u64 *titleIds, AM_PendingTitleInfo *titleInfo);

/**
 * @brief Deletes a pending title (both user and system titles are allowed).
 * @param mediatype Media type to delete from.
 * @param titleId ID of the pending title to delete.
 */
Result AM_DeletePendingTitle(FS_MediaType mediatype, u64 titleId);

/**
 * @brief Gets the number of active content imports for a given title.
 * @param[out] outNum Pointer to output the number of active content imports to.
 * @param titleId The title ID of the corresponding title.
 * @param mediaType The media type of the corresponding title.
 */
Result AM_GetNumImportContentContexts(u32 *outNum, u64 titleId, FS_MediaType mediaType);

/**
 * @brief Gets the content indices of the active content imports for a given title.
 * @param[out] outNum Pointer to output the number of read content indices to.
 * @param[out] outContentIndices Pointer to output the content indices to.
 * @param count The maximum number of content indices to retrieve.
 * @param titleId The title ID of the corresponding title.
 * @param mediaType The media type of the corresponding title.
 */
Result AM_GetImportContentContextList(u32 *outNum, u16 *outContentIndices, u32 count, u64 titleId, FS_MediaType mediaType);

/**
 * @brief Gets import contexts for a title's active content imports.
 * @param[out] outContexts Pointer to output the content import contexts to.
 * @param contentIndices The content indices of the contents to get import contexts for.
 * @param count The maximum number of import contexts to retrieve.
 * @param titleId The title ID of the corresponding title.
 * @param mediaType The media type of the corresponding title.
 */
Result AM_GetImportContentContexts(AM_ImportContentContext *outContexts, u16 *contentIndices, u32 count, u64 titleId, FS_MediaType mediaType);

/**
 * @brief Deletes active content import contexts for a given title.
 * @param contentIndices The content indices of the contents to delete import contexts for.
 * @param count The number of given content indices, or the number of content indices to delete.
 * @param titleId The title ID of the corresponding title.
 * @param mediaType The media type of the corresponding title.
 */
Result AM_DeleteImportContentContexts(u16 *contentIndices, u32 count, u64 titleId, FS_MediaType mediaType);

/**
 * @brief Checks if cleanup is required in the import database (when there are finalized titles left over in them). For NAND, this also returns true when nand:/data contains none or more than one directory, or when nand:/data/<ID0> does not exist.
 * @param[out] outNeedsCleanup Pointer to write the status to.
 * @param mediaType The media type to check.
 */
Result AM_NeedsCleanup(u8 *outNeedsCleanup, FS_MediaType mediaType);

/**
 * @brief Performs cleanup on the title import database, by deleting all titles found in the import.db that are marked as installed but haven't been transferred to the title.db. For NAND, if a file at nand:/data/disabled does not exist, this in addition removes any directory under nand:/data that doesn't match the system ID0.
 * @param mediaType The media type to perform cleanup on.
 */
Result AM_DoCleanup(FS_MediaType mediaType);

/**
 * @brief Deletes all pending titles (including both user and system titles).
 * @param mediatype Media type to delete from.
 */
Result AM_DeleteAllPendingTitles(FS_MediaType mediatype);

/// Deletes temporary titles.
Result AM_DeleteAllTemporaryTitles(void);

/**
 * @brief Initializes the external (SD) title database.
 * @param overwrite Overwrites the database if it already exists.
 */
Result AM_InitializeExternalTitleDatabase(bool overwrite);

/**
 * @brief Queries whether the external title database is available.
 * @param[out] available Pointer to output the availability status to.
 */
Result AM_QueryAvailableExternalTitleDatabase(bool* available);

/**
 * @brief Calculates the total file size of a DSiWare export.
 * @param[out] outSize Pointer to output the total file size to.
 * @param titleId The title ID of the corresponding DSiWare title.
 * @param exportType The export type to use when calculating the size.
 */
Result AM_CalculateTwlBackupSize(u64 *outSize, u64 titleId, u8 exportType);

/**
 * @brief Exports DSiWare to the specified filepath.
 * @param titleID TWL titleID.
 * @param exportType DSiWare export type.
 * @param workbuf Work buffer.
 * @param workbuf_size Work buffer size, must be >=0x20000.
 * @param filepath UTF-8 filepath(converted to UTF-16 internally).
 */
Result AM_ExportTwlBackup(u64 titleID, u8 exportType, void* workbuf, u32 workbuf_size, const char *filepath);

/**
 * @brief Imports DSiWare from the specified file.
 * @param filehandle FSUSER file handle.
 * @param exportType DSiWare export type.
 * @param buffer Work buffer.
 * @param size Buffer size, must be >=0x20000.
 */
Result AM_ImportTwlBackup(Handle filehandle, u8 exportType, void* buffer, u32 size);

/// Deletes all DSiWare (TWL) user titles (not system titles).
Result AM_DeleteAllTwlUserTitles(void);

/**
 * @brief Reads info from the specified DSiWare export file. This can only be used with DSiWare exported with certain export types.
 * @param filehandle FSUSER file handle.
 * @param[out] outinfo Output info buffer.
 * @param outinfo_size Output info buffer size.
 * @param workbuf Work buffer.
 * @param workbuf_size Work buffer size.
 * @param[out] banner Output banner buffer.
 * @param banner_size Output banner buffer size.
 */
Result AM_ReadTwlBackupInfo(Handle filehandle, void* outinfo, u32 outinfo_size, void* workbuf, u32 workbuf_size, void* banner, u32 banner_size);

/**
 * @brief Deletes all user titles (whether DSiWare or not) for which there are no tickets installed.
 * @param mediatype Media type to delete from.
 */
Result AM_DeleteAllExpiredTitles(FS_MediaType mediatype);

/**
 * @brief Retrieves information about the NAND TWL partition.
 * @param[out] info Pointer to output the TWL partition info to.
 */
Result AM_GetTWLPartitionInfo(AM_TWLPartitionInfo *info);

/**
 * @brief Retrieves ticket information for user-installed (not system titles or DLP child titles) whose console ID matches that of the system.
 * @param[out] outNum Pointer to output the number of retrieved ticket infos to.
 * @param[out] outInfos Pointer to output the ticket infos to.
 * @param count The maximum number of ticket infos to get.
 */
Result AM_GetPersonalizedTicketInfos(u32 *outNum, AM_TicketInfo *outInfos, u32 count);

/**
 * @brief Deletes all pending titles matching specific criteria.
 * @param mediaType The media type to delete the pending titles from.
 * @param flags The criteria based on which the pending titles should be deleted. @ref AM_PendingTitleFlags
 */
Result AM_DeleteAllImportTitleContexts(FS_MediaType mediaType, u32 flags);

/**
 * @brief Gets the number of pending titles matching specific criteria.
 * @param[out] outNum Pointer to output the number of pending titles to.
 * @param mediaType The media type to count the titles from.
 * @param flags The criteria based on which the pending titles should be counted. @ref AM_PendingTitleFlags
 */
Result AM_GetNumImportTitleContexts(u32 *outNum, FS_MediaType mediaType, u32 flags);

/**
 * @brief Gets a list of title IDs for pending titles matching specific criteria.
 * @param[out] outNum Pointer to output the number of pending titles to.
 * @param count The maximum number of title IDs to get.
 * @param flags The criteria based on which the title IDs should be retrieved. @ref AM_PendingTitleFlags
 * @param[out] titleIds Pointer to output the title IDs to.
 */
Result AM_GetImportTitleContextList(u32 *outNum, u32 count, FS_MediaType mediaType, u32 flags, u64 *titleIds);

/**
 * @brief Checks whether or not the user has the right to use a given content of a title.
 * @param[out] outHasRights Pointer to output whether or not the user has the right.
 * @param titleId Title ID of the title to perform the check for.
 * @param contentIndex Content index of the content to check.
 */
Result AM_CheckContentRights(u8 *outHasRights, u64 titleId, u16 contentIndex);

/**
 * @brief Obtains information about the limits section in tickets for the given list of title IDs.
 * @param[out] outInfos Pointer to output the ticket limit information to.
 * @param titleIds Pointer to the title IDs to obtain ticket limit information for.
 * @param count Maximum number of ticket limit infos to obtain.
 */
Result AM_GetTicketLimitInfo(AM_TicketLimitInfo *outInfos, u64 *titleIds, u32 count);

/**
 * @brief Obtains information about demo limits from AM's demo limit database.
 * @param[out] outInfos Pointer to output the demo limit information to.
 * @param titleIds Pointer to the title IDs to obtain demo limit information for.
 * @param count Maximum number of demo limit infos to obtain.
 */
Result AM_GetDemoLaunchInfo(AM_DemoLaunchInfo *outInfos, u64 *titleIds, u32 count);

/**
 * @brief Reads information from a DSiWare export file. Only works with exports that have 12 content sections (export types 7-11).
 * @param[out] outInfo Pointer to output the backup information to.
 * @param infoSize Size of the backup information buffer.
 * @param[out] outBanner Pointer to output the DSi banner to.
 * @param bannerSize Size of the banner buffer.
 * @param workingBuffer Pointer to use as a working buffer.
 * @param workingBufferSize Size of the working buffer.
 * @param backupFile File handle of the backup file.
 */
Result AM_ReadTwlBackupInfoEx(void *outInfo, u32 infoSize, void *outBanner, u32 bannerSize, void *workingBuffer, u32 workingBufferSize, Handle backupFile);

/**
 * @brief Deletes a batch of user titles (CTR or TWL system titles are not allowed here). The given title IDs must be installed to the same title database.
 * @param titleIds The title IDs of the titles to delete.
 * @param count The number of title IDs in the buffer.
 * @param mediaType The media type the titles are stored on.
 */
Result AM_DeleteUserTitles(u64 *titleIds, u32 count, FS_MediaType mediaType);

/**
 * @brief Returns the number of installed contents of a given title.
 * @param[out] outNum Pointer to output the number of installed contents to.
 * @param titleId Title ID of the corresponding title.
 * @param mediaType Media type of the corresponding title.
 */
Result AM_GetNumExistingContentInfos(u32 *outNum, u64 titleId, FS_MediaType mediaType);

/**
 * @brief Retrieves content information for installed contents of a given title.
 * @param[out] outNum Pointer to output the number of retrieved title infos to.
 * @param offset Offset of the title infos in the entire list.
 * @param count Number of title infos to retrive in this batch.
 * @param titleId Title ID of the corresponding title.
 * @param mediaType Media type of the corresponding title.
 */
Result AM_ListExistingContentInfos(u32 *outNum, AM_ContentInfo *outInfos, u32 offset, u32 count, u64 titleId, FS_MediaType mediaType);

/**
 * @brief Retrieves title information for a title, checking both the New3DS bit variant and the given title ID.
 * @param[out] outInfos Pointer to output the title information to.
 * @param titleIds Pointer to the title IDs to retrieve title information for.
 * @param count Number of given title IDs.
 * @param mediaType Media type of the given titles.
 */
Result AM_GetTitleInfosIgnorePlatform(AM_TitleInfo *outInfos, u64 *titleIds, u32 count, FS_MediaType mediaType);

/**
 * @brief Checks whether or not the user has the rights to use a content from a title, checking both the New3DS bit variant and the given title ID.
 * @param[out] outHasRights Pointer to output the status to.
 * @param titleId Title ID of the corresponding title.
 * @param contentIndex Content index of the content to check.
 */
Result AM_CheckContentRightsIgnorePlatform(u8 *outHasRights, u64 titleId, u16 contentIndex);


/**
 * @brief Installs a NATIVE_FIRM title to NAND. Accepts 0004013800000002 or 0004013820000002 (N3DS).
 * @param titleID Title ID of the NATIVE_FIRM to install.
 */
Result AM_InstallFirm(u64 titleID);

/**
 * @brief Initializes the CIA install process, returning a handle to write CIA data to.
 * @param mediatype Media type to install the CIA to.
 * @param[out] ciaHandle Pointer to write the CIA handle to.
 */
Result AM_StartCiaInstall(FS_MediaType mediatype, Handle *ciaHandle);

/**
 * @brief Initializes the CIA install process for temporary NAND titles (e.g. DLP child titles), returning a handle to write CIA data to.
 * @param[out] ciaHandle Pointer to write the CIA handle to.
 */
Result AM_StartTempCiaInstall(Handle *ciaHandle);

/**
 * @brief Aborts the CIA install process.
 * @param ciaHandle CIA handle to cancel.
 */
Result AM_CancelCIAInstall(Handle ciaHandle);

/**
 * @brief Finalizes the CIA install process.
 * @param ciaHandle CIA handle to finalize.
 */
Result AM_FinishCiaInstall(Handle ciaHandle);

/**
 * @brief Finalizes the CIA install process without committing the title to title.db or tmp*.db.
 * @param ciaHandle CIA handle to finalize.
 */
Result AM_FinishCiaInstallWithoutCommit(Handle ciaHandle);

/**
 * @brief Commits installed titles to the title database.
 * @param mediaType Location of the titles to commit.
 * @param titleCount Number of titles to commit.
 * @param temp Whether the titles being committed are in the temporary database.
 * @param titleIds Title IDs of the titles to commit.
 */
Result AM_CommitImportTitles(FS_MediaType mediaType, u32 titleCount, bool temp, const u64* titleIds);

/**
 * @brief Gets an AM_TitleInfo instance for a CIA file.
 * @param mediatype Media type that this CIA would be installed to.
 * @param[out] titleEntry Pointer to write the AM_TitleInfo instance to.
 * @param fileHandle Handle of the CIA file.
 */
Result AM_GetCiaFileInfo(FS_MediaType mediatype, AM_TitleInfo *titleEntry, Handle fileHandle);

/**
 * @brief Gets the SMDH icon data of a CIA file.
 * @param[out] icon Buffer to store the icon data in. Must be of size 0x36C0 bytes.
 * @param fileHandle Handle of the CIA file.
 */
Result AM_GetCiaIcon(void *icon, Handle fileHandle);

/**
 * @brief Gets the title ID dependency list of a CIA file.
 * @param[out] dependencies Buffer to store dependency title IDs in. Must be of size 0x300 bytes.
 * @param fileHandle Handle of the CIA file.
 */
Result AM_GetCiaDependencies(u64 *dependencies, Handle fileHandle);

/**
 * @brief Gets the meta section offset of a CIA file.
 * @param[out] metaOffset Pointer to output the meta section offset to.
 * @param fileHandle Handle of the CIA file.
 */
Result AM_GetCiaMetaOffset(u64 *metaOffset, Handle fileHandle);

/**
 * @brief Gets the core version of a CIA file.
 * @param[out] coreVersion Pointer to output the core version to.
 * @param fileHandle Handle of the CIA file.
 */
Result AM_GetCiaCoreVersion(u32 *coreVersion, Handle fileHandle);

/**
 * @brief Gets the free space, in bytes, required to install a CIA file.
 * @param[out] requiredSpace Pointer to output the required free space to.
 * @param mediaType Media type to check free space needed to install to.
 * @param fileHandle Handle of the CIA file.
 */
Result AM_GetCiaRequiredSpace(u64 *requiredSpace, FS_MediaType mediaType, Handle fileHandle);

/**
 * @brief Commits installed titles, and installs NATIVE_FIRM if necessary.
 * @param mediaType Location of the titles to finalize.
 * @param titleCount Number of titles to finalize.
 * @param temp Whether the titles being finalized are in the temporary database.
 * @param titleIds Title IDs to finalize.
 */
Result AM_CommitImportTitlesAndInstallFirmAuto(FS_MediaType mediaType, u32 titleCount, bool temp, u64* titleIds);

/// Installs the current NATIVE_FIRM title to NAND (firm0:/ & firm1:/)
Result AM_InstallFirmAuto(void);

/**
 * @brief Deletes a title.
 * @param mediatype Media type to delete from.
 * @param titleID ID of the title to delete.
 */
Result AM_DeleteTitle(FS_MediaType mediatype, u64 titleId);

/**
 * @brief Returns a list of TWL title IDs (0003XXXXXXXXXXXX) alongside the content IDs of the content with content index 0 for each.
 * @param[out] outNum Pointer to return the number of titles read.
 * @param[out] titleIds Pointer to output the TWL title IDs to.
 * @param[out] contentIds Pointer to output the content IDs to.
 * @param count The maximum number of titles to retrieve.
 */
Result AM_GetTwlTitleListForReboot(u32 *outNum, u64 *titleIds, u32 *contentIds, u32 count);

/**
 * @brief Returns the system updater mutex.
 * @param[out] outMutex Pointer to output the updater mutex handle to.
 */
Result AM_GetSystemUpdaterMutex(Handle *outMutex);

/**
 * @brief Returns the size of the metadata region in a CIA file.
 * @param[out] outSize Pointer to output the metadata region size to.
 * @param ciaFile Handle for the the CIA file.
 */
Result AM_GetCiaMetaSize(u32 *outSize, Handle ciaFile);

/**
 * @brief Gets the full meta section of a CIA file.
 * @param[out] meta Buffer to store the meta section in.
 * @param size Size of the buffer. Must be greater than or equal to the actual section data's size.
 * @param fileHandle Handle of the CIA file.
 */
Result AM_GetCiaMetaSection(void *meta, u32 size, Handle fileHandle);

/**
 * @brief Queries the AM demo launch database to check whether or not the user has the right to launch a demo. Note that this command will cause the demo play count to increase when there are remaining plays.
 * @param[out] outHasRight Pointer to output the status to.
 * @param titleId The title ID of the title to perform this check for.
 */
Result AM_CheckDemoLaunchRight(u8 *outHasRight, u64 titleId);

/**
 * @brief Returns internal title location info for a specific title.
 * @param[out] outInfo Pointer to output the info to.
 * @param titleId The title ID of the title to retrieve the info for.
 * @param mediaType The media type of the corresponding title.
 */
Result AM_GetInternalTitleLocationInfo(AM_InternalTitleLocationInfo *outInfo, u64 titleId, FS_MediaType mediaType);

/**
 * @brief Migrates the AGB_SAVE partition data to the .SAV format on the SD card.
 * @param titleId Title ID of the AGB title.
 * @param mediaType Media type of the AGB title.
 */
Result AM_MigrateAGBToSAV(u64 titleId, FS_MediaType mediaType);

/**
 * @brief Begins the installation of a CIA, set up to overwrite an existing title with the same title ID and media type.
 * @param[out] outCiaHandle Pointer to output the virtual import file handle to.
 * @param mediaType The target media of the title to be installed.
 */
Result AM_StartCiaInstallOverwrite(Handle *outCiaHandle, FS_MediaType mediaType);

/**
 * @brief Begins the installation of a CIA containing a system title.
 * @param[out] outCiaHandle Pointer to output the virtual import file handle to.
 */
Result AM_StartSystemCiaInstall(Handle *outCiaHandle);


/**
 * @brief Begins installing a ticket.
 * @param[out] ticketHandle Pointer to output a handle to write ticket data to.
 */
Result AMNET_InstallTicketBegin(Handle *ticketHandle);

/**
 * @brief Aborts installing a ticket.
 * @param ticketHandle Handle of the installation to abort.
 */
Result AMNET_InstallTicketAbort(Handle ticketHandle);

/**
 * @brief Finishes installing a ticket.
 * @param ticketHandle Handle of the installation to finalize.
 */
Result AMNET_InstallTicketFinish(Handle ticketHandle);

/**
 * @brief Begins installing a title.
 * @param mediaType Destination to install to.
 * @param titleId ID of the title to install.
 * @param inTempDb Whether or not the title should be installed to the temporary title database.
 */
Result AMNET_InstallTitleBegin(FS_MediaType mediaType, u64 titleId, bool inTempDb);

/// Pauses the currently active title installation.
Result AMNET_InstallTitleStop(void);

/**
 * @brief Resumes installing a title.
 * @param mediaType Destination to install to.
 * @param titleId ID of the title to install.
 */
Result AMNET_InstallTitleResume(FS_MediaType mediaType, u64 titleId);

/// Aborts installing a title.
Result AMNET_InstallTitleAbort(void);

/// Finishes installing a title.
Result AMNET_InstallTitleFinish(void);

/**
 * @brief Commits installed titles.
 * @param mediaType Location of the titles to finalize.
 * @param titleCount Number of titles to finalize.
 * @param temp Whether the titles being finalized are in the temporary database.
 * @param titleIds Title IDs to finalize.
 */
Result AMNET_CommitImportTitles(FS_MediaType mediaType, u32 titleCount, bool temp, const u64* titleIds);

/**
 * @brief Begins installing a TMD.
 * @param[out] tmdHandle Pointer to output a handle to write TMD data to.
 */
Result AMNET_InstallTmdBegin(Handle *tmdHandle);

/**
 * @brief Aborts installing a TMD.
 * @param tmdHandle Handle of the installation to abort.
 */
Result AMNET_InstallTmdAbort(Handle tmdHandle);

/**
 * @brief Finishes installing a TMD.
 * @param tmdHandle Handle of the installation to finalize.
 * @param unk Unknown - not used by AM or AMPXI.
 */
Result AMNET_InstallTmdFinish(Handle tmdHandle, bool unk);

/**
 * @brief Prepares to import title contents.
 * @param contentCount Number of contents to be imported.
 * @param contentIndices Indices of the contents to be imported.
 */
Result AMNET_CreateImportContentContexts(u32 contentCount, u16 *contentIndices);

/**
 * @brief Begins installing title content.
 * @param[out] contentHandle Pointer to output a handle to write content data to.
 * @param index Index of the content to install.
 */
Result AMNET_InstallContentBegin(Handle *contentHandle, u16 index);

/**
 * @brief Pauses the current content installation.
 * @param contentHandle Handle of the content installation to pause.
 */
Result AMNET_InstallContentStop(Handle contentHandle);

/**
 * @brief Resumes installing a content.
 * @param[out] contentHandle Pointer to output a handle to write content data to.
 * @param[out] resumeOffset Pointer to write the offset to resume content installation at to.
 * @param index Index of the content to install.
 */
Result AMNET_InstallContentResume(Handle *contentHandle, u64* resumeOffset, u16 index);

/**
 * @brief Cancels installing title content.
 * @param contentHandle Handle of the content installation to cancel.
 */
Result AMNET_InstallContentCancel(Handle contentHandle);

/**
 * @brief Finishes installing title content.
 * @param contentHandle Handle of the installation to finalize.
 */
Result AMNET_InstallContentFinish(Handle contentHandle);

/**
 * @brief Returns the number active content imports for the current title installation.
 * @param[out] outNum Pointer to output the number of content imports to.
 */
Result AMNET_GetNumCurrentImportContentContexts(u32 *outNum);

/**
 * @brief Returns the content indices of the active content imports for the current title installation.
 * @param[out] outNum Pointer to output the number of read content indices to.
 * @param[out] outIndices Pointer to output the content indices to.
 * @param count The maximum number of content indices to get.
 */
Result AMNET_GetCurrentImportContentContextsList(u32 *outNum, u16 *outIndices, u32 count);

/**
 * @brief Retrieves import content contexts for the active content imports of the current title installation.
 * @param[out] outContexts Pointer to output the import content contexts to.
 * @param indices Pointer to the content indices to retrieve import content contexts for.
 * @param count Number of content indices or maximum number of import content contexts to retrieve.
 */
Result AMNET_GetCurrentImportContentContexts(AM_ImportContentContext *outContexts, u16 *indices, u32 count);

/**
 * @brief Signs the given message with ECDSA with SHA256 using the device certificate (CTCert) using a randomly generated public key.
 * @param[out] outInternalResult Pointer to output an internal pxi:am9 result value to.
 * @param[out] outSignature Pointer to output the signature data to.
 * @param signatureSize The size of the output signature. This is usually 0x3C.
 * @param[out] outCertificate Pointer to output the certificate data to.
 * @param certificateSize The size of the output certificate. This is usually 0x180.
 * @param indata Pointer to the input message to sign.
 * @param indataSize Size of the input data.
 * @param titleId Title ID to use in the signing process.
 */
Result AMNET_Sign(s32 *outInternalResult, void *outSignature, u32 signatureSize, void *outCertificate, u32 certificateSize, void *indata, u32 indataSize, u64 titleId);

/**
 * @brief Returns the device certificate of the system (CTCert).
 * @param[out] outInternalResult Pointer to output an internal pxi:am9 result value to.
 * @param[out] outCert Pointer to output the device certificate data to.
 * @param certSize Size of the certificate. Must be 0x180.
 */
Result AMNET_GetCTCert(s32 *outInternalResult, void *outCert, u32 certSize);

/**
 * @brief Imports up to four certificates.
 * @param cert1Size Size of the first certificate.
 * @param cert1 Data of the first certificate.
 * @param cert2Size Size of the second certificate.
 * @param cert2 Data of the second certificate.
 * @param cert3Size Size of the third certificate.
 * @param cert3 Data of the third certificate.
 * @param cert4Size Size of the fourth certificate.
 * @param cert4 Data of the fourth certificate.
 */
Result AMNET_ImportCertificates(u32 cert1Size, void* cert1, u32 cert2Size, void* cert2, u32 cert3Size, void* cert3, u32 cert4Size, void* cert4);

/**
 * @brief Imports a certificate.
 * @param certSize Size of the certificate.
 * @param cert Data of the certificate.
 */
Result AMNET_ImportCertificate(u32 certSize, void* cert);

/**
 * @brief Commits pending titles, and installs NATIVE_FIRM if necessary.
 * @param mediaType Location of the titles to finalize.
 * @param titleCount Number of titles to finalize.
 * @param temp Whether the titles being finalized are in the temporary import database.
 * @param titleIds Title IDs to finalize.
 */
Result AMNET_CommitImportTitlesAndInstallFirmAuto(FS_MediaType mediaType, u32 titleCount, bool temp, u64 *titleIds);

/**
 * @brief Deletes a title's ticket matching a certain ticket ID.
 * @param titleId Title ID of the title.
 * @param ticketId The ticket ID of the ticket to delete for the title.
 */
Result AMNET_DeleteTicket(u64 titleId, u64 ticketId);

/**
 * @brief Gets the number of ticket IDs for a certain title (a given title can have more than one ticket ID).
 * @param[out] outNum Pointer to output the number of ticket IDs to.
 * @param titleId Title ID of the title to retrieve the number of ticket IDs for.
 */
Result AMNET_GetTitleNumTicketIds(u32 *outNum, u64 titleId);

/**
 * @brief Retrieves the ticket IDs for all installed tickets of a title.
 * @param[out] outNum Pointer to output the number of read ticket IDs to.
 * @param[out] outTicketIds Pointer to output the ticket IDs to.
 * @param count The maximum number of ticket IDs to retrieve.
 * @param titleId The title ID of the title to retrieve ticket IDs for.
 * @param verifyTickets Whether or not to verify the installed tickets as they're being read.
 */
Result AMNET_GetTitleTicketIds(u32 *outNum, u64 *outTicketIds, u32 count, u64 titleId, bool verifyTickets);

/**
 * @brief Gets the number of tickets for a certain title (a given title can have more than one ticket).
 * @param[out] outNum Pointer to output the number of tickets to.
 * @param titleId Title ID of the title to retrieve the number of tickets for.
 */
Result AMNET_GetTitleNumTickets(u32 *outNum, u64 titleId);

/**
 * @brief Retrieves information about a title's installed tickets (for any titles).
 * @param[out] outNum Pointer to output the number of retrieved ticket infos to.
 * @param[out] outInfos Pointer to output the ticket infos to.
 * @param offset Offset of the ticket infos to start from within the entire list.
 * @param count Maximum number of ticket infos to retrieve.
 * @param titleId Title ID of the corresponding title.
 */
Result AMNET_GetTicketInfos(u32 *outNum, AM_TicketInfo *outInfos, u32 offset, u32 count, u64 titleId);

/**
 * @brief Exports a license ticket's raw (non-encrypted) data.
 * @param[out] outActualSize Pointer to output the actual ticket size to.
 * @param[out] outdata Pointer to output the raw license ticket data to.
 * @param outdataSize Size of the output data buffer.
 * @param titleId Ticket title ID of the ticket to export.
 * @param ticketId Ticket ID of the license ticket to export.
 */
Result AMNET_ExportLicenseTicket(u32 *outActualSize, void *outdata, u32 outdataSize, u64 titleId, u64 ticketId);

/**
 * @brief Returns the number of contents in the TMD of the currently active title installation.
 * @param[out] outNum Pointer to output the number of contents to.
 */
Result AMNET_GetNumCurrentContentInfos(u32 *outNum);

/**
 * @brief Finds content infos of the current content installation's TMD using the given content indices.
 * @param[out] outInfos Pointer to output the content infos to.
 * @param contentIndices Pointer to the content indices to look for.
 * @param count Number of content indices.
 */
Result AMNET_FindCurrentContentInfos(AM_ContentInfo *outInfos, u16 *contentIndices, u32 count);

/**
 * @brief Retrieves content infos from the current content installation's TMD.
 * @param[out] outNum Pointer to output the number of retrieved content infos to.
 * @param[out] outInfos Pointer to output the content infos to.
 * @param offset Offset to start from in the entire content info list.
 * @param count Maximum number of content infos to retrieve.
 */
Result AMNET_ListCurrentContentInfos(u32 *outNum, AM_ContentInfo *outInfos, u32 offset, u32 count);

/**
 * @brief Calculates the total storage space required to install the provided optional (@ref AM_ContentTypeFlags) contents (e.g. for DLC).
 * @param[out] outRequiredSize Pointer to output the total required size (in bytes) to.
 * @param contentIndices Pointer to the input content indices for the optional contents to include in the size calculation.
 * @param count Number of content indices.
 * @param titleId Title ID of the corresponding title.
 * @param mediaType Media type of the corresponding title.
 */
Result AMNET_CalculateContextRequiredSize(u64 *outRequiredSize, u16 *contentIndices, u32 count, u64 titleId, FS_MediaType mediaType);

/**
 * @brief Similar to @ref AMNET_CreateImportContentContexts, but this command must be called after finializing the TMD installation but not while a content import is active. This command recreates the current title installation's import.db entry, and then proceeds to create new import content contexts for the given content indices.
 */
Result AMNET_UpdateImportContentContexts(u16 *contentIndices, u32 count);

/// Resets play count of all installed demos by deleting their launch info.
Result AMNET_DeleteAllDemoLaunchInfos(void);

/**
 * @brief Begins a new title installation in overwrite mode.
 * @param titleId Title ID of the title.
 * @param mediaType Media type of the title.
 */
Result AMNET_InstallTitleBeginForOverwrite(u64 titleId, FS_MediaType mediaType);

/**
 * @brief Exports an encrypted buffer containing a ticket to be sent to the CDN for title downloads.
 * @param[out] outWrappedTicketSize Pointer to output the actual size of the encrypted ticket data to.
 * @param[out] outKeyIvSize Pointer to output the actual size of the encryption key and IV to.
 * @param[out] outWrappedTicket Pointer to output the encrypted ticket data to.
 * @param wrappedTicketSize Size of the output ticket data buffer.
 * @param[out] outKeyIv Pointer to output the encryption key and IV to.
 * @param keyIvSize Size of the encryption key and IV buffer.
 * @param titleId Title ID of the corresponding title.
 * @param ticketId Ticket ID of the ticket to export.
 */
Result AMNET_ExportTicketWrapped(u32 *outWrappedTicketSize, u32 *outKeyIvSize, void *outWrappedTicket, u32 wrappedTicketSize, void *outKeyIv, u32 keyIvSize, u64 titleId, u64 ticketId);


/**
 * @brief Gets the number of contents in the TMD of the specified DLC title.
 * @param[out] count Pointer to output the number of contents to.
 * @param mediatype Media type of the title.
 * @param titleID Title ID to retrieve the count for.
 */
Result AMAPP_GetDLCContentInfoCount(u32* count, FS_MediaType mediatype, u64 titleID);

/**
 * @brief Finds content infos from the TMD of an installed DLC title.
 * @param[out] outInfos Pointer to output the found content infos to.
 * @param contentIndices Pointer to the content indices to find content infos for.
 * @param count Maximum number of content infos to retrieve.
 * @param titleId Title ID of the DLC title.
 * @param mediaType Media type of the DLC title.
 */
Result AMAPP_FindDLCContentInfos(AM_ContentInfo *outInfos, u16 *contentIndices, u32 count, u64 titleId, FS_MediaType mediaType);

/**
 * @brief Gets content infos from the TMD of the specified DLC title.
 * @param[out] contentInfoRead Pointer to output the number of content infos read to.
 * @param mediatype Media type of the title.
 * @param titleID Title ID to retrieve the content infos for.
 * @param contentInfoCount Number of content infos to retrieve.
 * @param offset Offset from the first content index the count starts at.
 * @param[out] contentInfos Pointer to output the content infos read to.
 */
Result AMAPP_ListDLCContentInfos(u32* contentInfoRead, FS_MediaType mediatype, u64 titleID, u32 contentInfoCount, u32 offset, AM_ContentInfo* contentInfos);

/**
 * @brief Deletes a range of contents from an installed DLC title.
 * @param contentIndices Pointer to the content indices of the contents to delete.
 * @param count Number of given content indices.
 * @param titleId Title ID of the DLC title to delete the contents from.
 * @param mediaType Media type of the DLC title.
 */
Result AMAPP_DeleteDLCContents(u16 *contentIndices, u32 count, u64 titleId, FS_MediaType mediaType);

/**
 * @brief Retrieves title information about installed DLC titles.
 * @param[out] outInfos Pointer to output the read title infos to.
 * @param titleIds Pointer to the input title IDs of the DLC titles to retrieve title infos for.
 * @param count Number of input DLC title IDs, or the maximum number of title infos to retrieve.
 * @param mediaType Media type of the given DLC titles.
 */
Result AMAPP_GetDLCTitleInfos(AM_TitleInfo *outInfos, u64 *titleIds, u32 count, FS_MediaType mediaType);

/**
 * @brief Returns the number of tickets installed for a given DLC or license title ID.
 * @param[out] outNum Pointer to output the number of installed tickets to.
 * @param titleId The input DLC or license ticket title ID.
 */
Result AMAPP_GetDLCOrLicenseNumTickets(u32 *outNum, u64 titleId);

/**
 * @brief Returns information about installed DLC or license tickets.
 * @param[out] outNum Pointer to output the number of retrieved ticket infos to.
 * @param[out] outInfos Pointer to output the retrieved ticket infos to.
 * @param offset Offset to start from in the entire ticket infos list.
 * @param count Maximum number of ticket infos to retrieve.
 * @param titleId The input DLC or license ticket title ID.
 */
Result AMAPP_ListDLCOrLicenseTicketInfos(u32 *outNum, AM_TicketInfo *outInfos, u32 offset, u32 count, u64 titleId);

/**
 * @brief Retrieves rights data from a DLC or license ticket. This always reads at most rightsSize / (size of one record depending on rightsType).
 * @param[out] outNumRecords Pointer to output the total number of records in the ticket corresponding to the given rights type.
 * @param[out] outNextOffset Pointer to output the next offset to for iterating through the records.
 * @param[out] outRights Pointer to output the rights records to.
 * @param rightsSize Size of the rights record buffer.
 * @param rightsType The type of rights record to retrieve. Allowed values range from 1 to 6, inclusive.
 * @param recordOffset The record offset to start from in the entire rights record list of the given type.
 * @param titleId The ticket title ID of the ticket to read.
 * @param ticketId The ticket ID of the ticket to read.
 */
Result AMAPP_GetDLCOrLicenseItemRights(u32 *outNumRecords, u32 *outNextOffset, void *outRights, u32 rightsSize, u32 rightsType, u32 recordOffset, u64 titleId, u64 ticketId);

/**
 * @brief Returns whether or not the given DLC title is being accessed by one or more processes.
 * @param[out] outInUse Pointer to output the status to.
 * @param titleId The title ID of the DLC title to check.
 * @param mediaType The media type of the DLC title to check.
 */
Result AMAPP_IsDLCTitleInUse(u8 *outInUse, u64 titleId, FS_MediaType mediaType);

/**
 * @brief Queries whether the SD title database is available.
 * @param[out] available Pointer to output the availability status to.
 */
Result AMAPP_QueryAvailableExternalTitleDatabase(bool* available);

/**
 * @brief Returns the number of installed contents for a DLC title.
 * @param[out] outNum Pointer to output the number of installed contents to.
 * @param titleId The title ID of the DLC title.
 * @param mediaType Media type of the DLC title.
 */
Result AMAPP_GetNumExistingDLCContentInfos(u32 *outNum, u64 titleId, FS_MediaType mediaType);

/**
 * @brief Retrieves information about the installed contents for a DLC title.
 * @param[out] outNum Pointer to output the number of retrieved content infos to.
 * @param[out] outInfos Pointer to output the content infos to.
 * @param offset Offset to start from in the entire content info list.
 * @param count Maximum number of content infos to retrieve.
 * @param titleId The title ID of the DLC title.
 * @param mediaType The media type of the DLC title.
 */
Result AMAPP_ListExistingDLCContentInfos(u32 *outNum, AM_ContentInfo *outInfos, u32 offset, u32 count, u64 titleId, FS_MediaType mediaType);

/**
 * @brief Returns information about installed patch (update) titles.
 * @param[out] outInfos Pointer to output the title infos to.
 * @param titleIds The title IDs of the patch titles to retrieve title infos for.
 * @param count The number of given title IDs.
 * @param mediaType The media type of the given titles.
 */
Result AMAPP_GetPatchTitleInfos(AM_TitleInfo *outInfos, u64 *titleIds, u32 count, FS_MediaType mediaType);
