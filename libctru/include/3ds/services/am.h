/**
 * @file am.h
 * @brief AM (Application Manager) service.
 */
#pragma once

/**
 * @brief Contains basic information about a title.
 */
typedef struct
{
	u64 titleID; ///< The title's ID.
	u64 size;    ///< The title's installed size.
	u16 version; ///< The title's version.
	u8 unk[6];   ///< Unknown title data.
} AM_TitleEntry;

/// Initializes AM.
Result amInit(void);

/// Exits AM.
void amExit(void);

/// Gets the current AM session handle.
Handle *amGetSessionHandle(void);

/**
 * @brief Gets the number of titles for a given mediatype.
 * @param mediatype Mediatype to get titles from.
 * @param count Pointer to write the title count to.
 */
Result AM_GetTitleCount(u8 mediatype, u32 *count);

/**
 * @brief Gets a list of title IDs present in a mediatype.
 * @param mediatype Mediatype to get titles from.
 * @param count Number of title IDs to get.
 * @param titleIDs Buffer to write retrieved title IDs to.
 */
Result AM_GetTitleIdList(u8 mediatype, u32 count, u64 *titleIDs);

/**
 * @brief Gets a 32-bit device-specific ID.
 * @param deviceID Pointer to write the device ID to.
 */
Result AM_GetDeviceId(u32 *deviceID);

/**
 * @brief Gets a list of details about installed titles.
 * @param mediatype Mediatype to get titles from.
 * @param titleCount Number of titles to list.
 * @param titleIdList List of title IDs to retrieve details for.
 * @param titleList Buffer to write AM_TitleEntry's to.
 */
Result AM_ListTitles(u8 mediatype, u32 titleCount, u64 *titleIdList, AM_TitleEntry *titleList);

/**
 * @brief Initializes the CIA install process, returning a handle to write CIA data to.
 * @param mediatype Mediatype to install the CIA to.
 * @param ciaHandle Pointer to write the CIA handle to.
 */
Result AM_StartCiaInstall(u8 mediatype, Handle *ciaHandle);

/**
 * @brief Initializes the CIA install process for Download Play CIAs, returning a handle to write CIA data to.
 * @param ciaHandle Pointer to write the CIA handle to.
 */
Result AM_StartDlpChildCiaInstall(Handle *ciaHandle);

/**
 * @brief Aborts the CIA install process.
 * @param ciaHandle Pointer to the CIA handle to cancel.
 */
Result AM_CancelCIAInstall(Handle *ciaHandle);

/**
 * @brief Finalizes the CIA install process.
 * @param mediatype Mediatype to install the CIA to.
 * @param ciaHandle Pointer to the CIA handle to finalize.
 */
Result AM_FinishCiaInstall(u8 mediatype, Handle *ciaHandle);

/**
 * @brief Deletes a title.
 * @param mediatype Mediatype to delete from.
 * @param titleID ID of the title to delete.
 */
Result AM_DeleteTitle(u8 mediatype, u64 titleID);

/**
 * @brief Deletes a title, provided that it is not a system title.
 * @param mediatype Mediatype to delete from.
 * @param titleID ID of the title to delete.
 */
Result AM_DeleteAppTitle(u8 mediatype, u64 titleID);

/// Installs the current NATIVE_FIRM title to NAND (firm0:/ & firm1:/)
Result AM_InstallNativeFirm(void);

/// Similar to InstallNativeFirm, but doesn't use AMPXI_GetTitleList (NATIVE_FIRM: 0004013800000002 or 0004013820000002 (N3DS))
Result AM_InstallFirm(u64 titleID);

/**
 * @brief Gets the product code of a title.
 * @param mediatype Mediatype of the title.
 * @param titleID ID of the title.
 * @param productCode Buffer to output the product code to. (length = 16)
 */
Result AM_GetTitleProductCode(u8 mediatype, u64 titleID, char* productCode);

/**
 * @brief Gets an AM_TitleEntry instance for a CIA file.
 * @param mediatype Mediatype that this CIA would be installed to.
 * @param titleEntry Pointer to write the AM_TitleEntry instance to.
 * @param fileHandle Handle of the CIA file to read.
 */
Result AM_GetCiaFileInfo(u8 mediatype, AM_TitleEntry *titleEntry, Handle fileHandle);

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
