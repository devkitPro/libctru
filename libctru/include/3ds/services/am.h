/**
 * @file am.h
 * @brief AM (Application Manager) service.
 */
#pragma once

#include <3ds/services/fs.h>

/// Contains basic information about a title.
typedef struct
{
	u64 titleID; ///< The title's ID.
	u64 size;    ///< The title's installed size.
	u16 version; ///< The title's version.
	u8 unk[6];   ///< Unknown title data.
} AM_TitleEntry;

/// Pending title status mask values.
enum
{
	AM_STATUS_MASK_INSTALLING = BIT(0),           ///< Titles currently installing.
	AM_STATUS_MASK_AWAITING_FINALIZATION = BIT(1) ///< Titles awaiting finalization.
};

/// Pending title status values.
typedef enum
{
	AM_STATUS_ABORTED = 0x0002,              ///< Install aborted.
	AM_STATUS_SAVED = 0x0003,                ///< Title saved, but not installed.
	AM_STATUS_INSTALL_IN_PROGRESS = 0x0802,  ///< Install in progress.
	AM_STATUS_AWAITING_FINALIZATION = 0x0803 ///< Awaiting finalization.
} AM_InstallStatus;

// Contains basic information about a pending title.
typedef struct
{
	u64 titleId;   ///< Title ID
	u16 version;   ///< Version
	u16 status;    ///< @ref AM_InstallStatus
	u32 titleType; ///< Title Type
	u8 unk[0x8];   ///< Unknown
} AM_PendingTitleEntry;

/// Pending title deletion flags.
enum
{
	AM_DELETE_PENDING_NON_SYSTEM = BIT(0), ///< Non-system titles.
	AM_DELETE_PENDING_SYSTEM = BIT(1)      ///< System titles.
};

/// Information about the TWL NAND partition.
typedef struct {
	u64 capacity;        ///< Total capacity.
	u64 freeSpace;       ///< Total free space.
	u64 titlesCapacity;  ///< Capacity for titles.
	u64 titlesFreeSpace; ///< Free space for titles.
} AM_TWLPartitionInfo;

/// Initializes AM. This doesn't initialize with "am:app", see amAppInit().
Result amInit(void);

/// Initializes AM with a service which has access to the amapp-commands. This should only be used when using the amapp commands, not non-amapp AM commands.
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
 * @param titleIds Buffer to output the retrieved title IDs to.
 */
Result AM_GetTitleList(u32* titlesRead, FS_MediaType mediatype, u32 titleCount, u64 *titleIds);

/**
 * @brief Gets a list of details about installed titles.
 * @param mediatype Media type to get titles from.
 * @param titleCount Number of titles to list.
 * @param titleIds List of title IDs to retrieve details for.
 * @param titleInfo Buffer to write AM_TitleEntry's to.
 */
Result AM_GetTitleInfo(FS_MediaType mediatype, u32 titleCount, u64 *titleIds, AM_TitleEntry *titleInfo);

/**
 * @brief Gets the number of tickets installed on the system.
 * @param[out] count Pointer to output the ticket count to.
 */
Result AM_GetTicketCount(u32 *count);

/**
 * @brief Gets a list of tickets installed on the system.
 * @param[out] ticketsRead Pointer to output the number of read tickets to.
 * @param ticketCount Number of tickets to read.
 * @param skip Number of tickets to skip.
 * @param ticketIds Buffer to output the retrieved ticket IDs to.
 */
Result AM_GetTicketList(u32 *ticketsRead, u32 ticketCount, u32 skip, u64 *ticketIds);

/**
 * @brief Gets the number of pending titles on this system.
 * @param[out] count Pointer to output the pending title count to.
 * @param mediatype Media type of pending titles to count.
 * @param statusMask Bit mask of status values to include.
 */
Result AM_GetPendingTitleCount(u32 *count, FS_MediaType mediatype, u32 statusMask);

/**
 * @brief Gets a list of pending titles on this system.
 * @param[out] titlesRead Pointer to output the number of read pending titles to.
 * @param titleCount Number of pending titles to read.
 * @param mediatype Media type of pending titles to list.
 * @param statusMask Bit mask of status values to include.
 * @param titleIds Buffer to output the retrieved pending title IDs to.
 */
Result AM_GetPendingTitleList(u32 *titlesRead, u32 titleCount, FS_MediaType mediatype, u32 statusMask, u64 *titleIds);

/**
 * @brief Gets information about pending titles on this system.
 * @param titleCount Number of pending titles to read.
 * @param mediatype Media type of pending titles to get information on.
 * @param titleIds IDs of the titles to get information about.
 * @param titleInfo Buffer to output the retrieved pending title info to.
 */
Result AM_GetPendingTitleInfo(u32 titleCount, FS_MediaType mediatype, u64 *titleIds, AM_PendingTitleEntry *titleInfo);

/**
 * @brief Gets a 32-bit device-specific ID.
 * @param deviceID Pointer to write the device ID to.
 */
Result AM_GetDeviceId(u32 *deviceID);

/**
 * @brief Exports DSiWare to the specified filepath.
 * @param titleID TWL titleID.
 * @param operation DSiWare operation type.
 * @param workbuf Work buffer.
 * @param workbuf_size Work buffer size, must be >=0x20000.
 * @param filepath UTF-8 filepath(converted to UTF-16 internally).
 */
Result AM_ExportTwlBackup(u64 titleID, u8 operation, void* workbuf, u32 workbuf_size, const char *filepath);

/**
 * @brief Imports DSiWare from the specified file.
 * @param filehandle FSUSER file handle.
 * @param operation DSiWare operation type.
 * @param buffer Work buffer.
 * @param size Buffer size, must be >=0x20000.
 */
Result AM_ImportTwlBackup(Handle filehandle, u8 operation, void* buffer, u32 size);

/**
 * @brief Reads info from the specified DSiWare export file. This can only be used with DSiWare exported with certain operation value(s).
 * @param filehandle FSUSER file handle.
 * @param outinfo Output info buffer.
 * @param outinfo_size Output info buffer size.
 * @param workbuf Work buffer.
 * @param workbuf_size Work buffer size.
 * @param banner Output banner buffer.
 * @param banner_size Output banner buffer size.
 */
Result AM_ReadTwlBackupInfo(Handle filehandle, void* outinfo, u32 outinfo_size, void* workbuf, u32 workbuf_size, void* banner, u32 banner_size);

/**
 * @brief Retrieves information about the NAND TWL partition.
 * @param[out] info Pointer to output the TWL partition info to.
 */
Result AM_GetTWLPartitionInfo(AM_TWLPartitionInfo *info);

/**
 * @brief Initializes the CIA install process, returning a handle to write CIA data to.
 * @param mediatype Media type to install the CIA to.
 * @param[out] ciaHandle Pointer to write the CIA handle to.
 */
Result AM_StartCiaInstall(FS_MediaType mediatype, Handle *ciaHandle);

/**
 * @brief Initializes the CIA install process for Download Play CIAs, returning a handle to write CIA data to.
 * @param[out] ciaHandle Pointer to write the CIA handle to.
 */
Result AM_StartDlpChildCiaInstall(Handle *ciaHandle);

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
 * @brief Commits installed CIAs.
 * @param mediaType Location of the titles to finalize.
 * @param titleCount Number of titles to finalize.
 * @param temp Whether the titles being finalized are in the temporary database.
 * @param titleIds Title IDs to finalize.
 */
Result AM_CommitImportPrograms(FS_MediaType mediaType, u32 titleCount, bool temp, const u64* titleIds);

/**
 * @brief Deletes a title.
 * @param mediatype Media type to delete from.
 * @param titleID ID of the title to delete.
 */
Result AM_DeleteTitle(FS_MediaType mediatype, u64 titleID);

/**
 * @brief Deletes a title, provided that it is not a system title.
 * @param mediatype Media type to delete from.
 * @param titleID ID of the title to delete.
 */
Result AM_DeleteAppTitle(FS_MediaType mediatype, u64 titleID);

/**
 * @brief Deletes a ticket.
 * @param titleID ID of the ticket to delete.
 */
Result AM_DeleteTicket(u64 ticketId);

/**
 * @brief Deletes a pending title.
 * @param mediatype Media type to delete from.
 * @param titleId ID of the pending title to delete.
 */
Result AM_DeletePendingTitle(FS_MediaType mediatype, u64 titleId);

/**
 * @brief Deletes pending titles.
 * @param mediatype Media type to delete from.
 * @param flags Flags used to select pending titles.
 */
Result AM_DeletePendingTitles(FS_MediaType mediatype, u32 flags);

/**
 * @brief Deletes all pending titles.
 * @param mediatype Media type to delete from.
 */
Result AM_DeleteAllPendingTitles(FS_MediaType mediatype);

/// Installs the current NATIVE_FIRM title to NAND (firm0:/ & firm1:/)
Result AM_InstallNativeFirm(void);

/**
 * @brief Installs a NATIVE_FIRM title to NAND. Accepts 0004013800000002 or 0004013820000002 (N3DS).
 * @param titleID Title ID of the NATIVE_FIRM to install.
 */
Result AM_InstallFirm(u64 titleID);

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
 * @brief Gets an AM_TitleEntry instance for a CIA file.
 * @param mediatype Media type that this CIA would be installed to.
 * @param[out] titleEntry Pointer to write the AM_TitleEntry instance to.
 * @param fileHandle Handle of the CIA file.
 */
Result AM_GetCiaFileInfo(FS_MediaType mediatype, AM_TitleEntry *titleEntry, Handle fileHandle);

/**
 * @brief Gets the SMDH icon data of a CIA file.
 * @param icon Buffer to store the icon data in. Must be of size 0x36C0 bytes.
 * @param fileHandle Handle of the CIA file.
 */
Result AM_GetCiaIcon(void *icon, Handle fileHandle);

/**
 * @brief Gets the title ID dependency list of a CIA file.
 * @param dependencies Buffer to store dependency title IDs in. Must be of size 0x300 bytes.
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
 * @brief Gets the full meta section of a CIA file.
 * @param meta Buffer to store the meta section in.
 * @param size Size of the buffer. Must be greater than or equal to the actual section data's size.
 * @param fileHandle Handle of the CIA file.
 */
Result AM_GetCiaMetaSection(void *meta, u32 size, Handle fileHandle);

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
 * @brief Begins installing a ticket.
 * @param[out] ticketHandle Pointer to output a handle to write ticket data to.
 */
Result AM_InstallTicketBegin(Handle *ticketHandle);

/**
 * @brief Aborts installing a ticket.
 * @param ticketHandle Handle of the installation to abort.
 */
Result AM_InstallTicketAbort(Handle ticketHandle);

/**
 * @brief Finishes installing a ticket.
 * @param ticketHandle Handle of the installation to finalize.
 */
Result AM_InstallTicketFinish(Handle ticketHandle);

/**
 * @brief Begins installing a title.
 * @param mediaType Destination to install to.
 * @param titleId ID of the title to install.
 * @param unk Unknown. (usually false)
 */
Result AM_InstallTitleBegin(FS_MediaType mediaType, u64 titleId, bool unk);

/// Stops installing a title, generally to be resumed later.
Result AM_InstallTitleStop(void);

/**
 * @brief Resumes installing a title.
 * @param mediaType Destination to install to.
 * @param titleId ID of the title to install.
 */
Result AM_InstallTitleResume(FS_MediaType mediaType, u64 titleId);

/// Aborts installing a title.
Result AM_InstallTitleAbort(void);

/// Finishes installing a title.
Result AM_InstallTitleFinish(void);

/**
 * @brief Commits installed titles.
 * @param mediaType Location of the titles to finalize.
 * @param titleCount Number of titles to finalize.
 * @param temp Whether the titles being finalized are in the temporary database.
 * @param titleIds Title IDs to finalize.
 */
Result AM_CommitImportTitles(FS_MediaType mediaType, u32 titleCount, bool temp, const u64* titleIds);

/**
 * @brief Begins installing a TMD.
 * @param[out] tmdHandle Pointer to output a handle to write TMD data to.
 */
Result AM_InstallTmdBegin(Handle *tmdHandle);

/**
 * @brief Aborts installing a TMD.
 * @param tmdHandle Handle of the installation to abort.
 */
Result AM_InstallTmdAbort(Handle tmdHandle);

/**
 * @brief Finishes installing a TMD.
 * @param tmdHandle Handle of the installation to finalize.
 * @param unk Unknown. (usually true)
 */
Result AM_InstallTmdFinish(Handle tmdHandle, bool unk);

/**
 * @brief Prepares to import title contents.
 * @param contentCount Number of contents to be imported.
 * @param contentIndices Indices of the contents to be imported.
 */
Result AM_CreateImportContentContexts(u32 contentCount, u16* contentIndices);

/**
 * @brief Begins installing title content.
 * @param[out] contentHandle Pointer to output a handle to write content data to.
 * @param index Index of the content to install.
 */
Result AM_InstallContentBegin(Handle *contentHandle, u16 index);

/**
 * @brief Stops installing title content, generally to be resumed later.
 * @param contentHandle Handle of the installation to abort.
 */
Result AM_InstallContentStop(Handle contentHandle);

/**
 * @brief Resumes installing title content.
 * @param[out] contentHandle Pointer to output a handle to write content data to.
 * @param[out] resumeOffset Pointer to write the offset to resume content installation at to.
 * @param index Index of the content to install.
 */
Result AM_InstallContentResume(Handle *contentHandle, u64* resumeOffset, u16 index);

/**
 * @brief Cancels installing title content.
 * @param contentHandle Handle of the installation to finalize.
 */
Result AM_InstallContentCancel(Handle contentHandle);

/**
 * @brief Finishes installing title content.
 * @param contentHandle Handle of the installation to finalize.
 */
Result AM_InstallContentFinish(Handle contentHandle);

/**
 * @brief Imports up to four certificates into the ticket certificate chain.
 * @param cert1Size Size of the first certificate.
 * @param cert1 Data of the first certificate.
 * @param cert2Size Size of the second certificate.
 * @param cert2 Data of the second certificate.
 * @param cert3Size Size of the third certificate.
 * @param cert3 Data of the third certificate.
 * @param cert4Size Size of the fourth certificate.
 * @param cert4 Data of the fourth certificate.
 */
Result AM_ImportCertificates(u32 cert1Size, void* cert1, u32 cert2Size, void* cert2, u32 cert3Size, void* cert3, u32 cert4Size, void* cert4);

/**
 * @brief Imports a certificate into the ticket certificate chain.
 * @param certSize Size of the certificate.
 * @param cert Data of the certificate.
 */
Result AM_ImportCertificate(u32 certSize, void* cert);

/**
 * @brief Commits installed titles, and updates FIRM if necessary.
 * @param mediaType Location of the titles to finalize.
 * @param titleCount Number of titles to finalize.
 * @param temp Whether the titles being finalized are in the temporary database.
 * @param titleIds Title IDs to finalize.
 */
Result AM_CommitImportTitlesAndUpdateFirmwareAuto(FS_MediaType mediaType, u32 titleCount, bool temp, u64* titleIds);

/// Resets play count of all installed demos by deleting their launch info.
Result AM_DeleteAllDemoLaunchInfos(void);

/// Deletes temporary titles.
Result AM_DeleteAllTemporaryTitles(void);

/**
 * @brief Deletes all expired titles.
 * @param mediatype Media type to delete from.
 */
Result AM_DeleteAllExpiredTitles(FS_MediaType mediatype);

/// Deletes all TWL titles.
Result AM_DeleteAllTwlTitles(void);
