/**
 * @file fs.h
 * @brief Filesystem Services
 */
#pragma once

#include <3ds/types.h>

/// Open flags.
enum
{
	FS_OPEN_READ   = BIT(0), ///< Open for reading.
	FS_OPEN_WRITE  = BIT(1), ///< Open for writing.
	FS_OPEN_CREATE = BIT(2), ///< Create file.
};

/// Write flags.
enum
{
	FS_WRITE_FLUSH       = BIT(0), ///< Flush.
	FS_WRITE_UPDATE_TIME = BIT(8), ///< Update file timestamp.
};

/// Attribute flags.
enum
{
	FS_ATTRIBUTE_DIRECTORY = BIT(0),  ///< Directory.
	FS_ATTRIBUTE_HIDDEN    = BIT(8),  ///< Hidden.
	FS_ATTRIBUTE_ARCHIVE   = BIT(16), ///< Archive.
	FS_ATTRIBUTE_READ_ONLY = BIT(24), ///< Read-only.
};

/// Media types.
typedef enum
{
	MEDIATYPE_NAND      = 0, ///< NAND.
	MEDIATYPE_SD        = 1, ///< SD card.
	MEDIATYPE_GAME_CARD = 2, ///< Game card.
} FS_MediaType;

/// System media types.
typedef enum
{
	SYSTEM_MEDIATYPE_CTR_NAND  = 0, ///< CTR NAND.
	SYSTEM_MEDIATYPE_TWL_NAND  = 1, ///< TWL NAND.
	SYSTEM_MEDIATYPE_SD        = 2, ///< SD card.
	SYSTEM_MEDIATYPE_TWL_PHOTO = 3, ///< TWL Photo.
} FS_SystemMediaType;

/// Archive IDs.
typedef enum
{
	ARCHIVE_ROMFS                    = 0x00000003, ///< RomFS archive.
	ARCHIVE_SAVEDATA                 = 0x00000004, ///< Save data archive.
	ARCHIVE_EXTDATA                  = 0x00000006, ///< Ext data archive.
	ARCHIVE_SHARED_EXTDATA           = 0x00000007, ///< Shared ext data archive.
	ARCHIVE_SYSTEM_SAVEDATA          = 0x00000008, ///< System save data archive.
	ARCHIVE_SDMC                     = 0x00000009, ///< SDMC archive.
	ARCHIVE_SDMC_WRITE_ONLY          = 0x0000000A, ///< Write-only SDMC archive.
	ARCHIVE_BOSS_EXTDATA             = 0x12345678, ///< BOSS ext data archive.
	ARCHIVE_CARD_SPIFS               = 0x12345679, ///< Card SPI FS archive.
	ARCHIVE_EXTDATA_AND_BOSS_EXTDATA = 0x1234567B, ///< Ext data and BOSS ext data archive.
	ARCHIVE_SYSTEM_SAVEDATA2         = 0x1234567C, ///< System save data archive.
	ARCHIVE_NAND_RW                  = 0x1234567D, ///< Read-write NAND archive.
	ARCHIVE_NAND_RO                  = 0x1234567E, ///< Read-only NAND archive.
	ARCHIVE_NAND_RO_WRITE_ACCESS     = 0x1234567F, ///< Read-only write access NAND archive.
	ARCHIVE_SAVEDATA_AND_CONTENT     = 0x2345678A, ///< User save data and ExeFS/RomFS archive.
	ARCHIVE_SAVEDATA_AND_CONTENT2    = 0x2345678E, ///< User save data and ExeFS/RomFS archive (only ExeFS for fs:LDR).
	ARCHIVE_NAND_CTR_FS              = 0x567890AB, ///< NAND CTR FS archive.
	ARCHIVE_TWL_PHOTO                = 0x567890AC, ///< TWL PHOTO archive.
	ARCHIVE_TWL_SOUND                = 0x567890AD, ///< TWL SOUND archive.
	ARCHIVE_NAND_TWL_FS              = 0x567890AE, ///< NAND TWL FS archive.
	ARCHIVE_NAND_W_FS                = 0x567890AF, ///< NAND W FS archive.
	ARCHIVE_GAMECARD_SAVEDATA        = 0x567890B1, ///< Game card save data archive.
	ARCHIVE_USER_SAVEDATA            = 0x567890B2, ///< User save data archive.
	ARCHIVE_DEMO_SAVEDATA            = 0x567890B4, ///< Demo save data archive.
} FS_ArchiveID;

/// Path types.
typedef enum
{
	PATH_INVALID = 0, ///< Invalid path.
	PATH_EMPTY   = 1, ///< Empty path.
	PATH_BINARY  = 2, ///< Binary path. Meaning is per-archive.
	PATH_ASCII   = 3, ///< ASCII text path.
	PATH_UTF16   = 4, ///< UTF-16 text path.
} FS_PathType;

/// Secure value slot.
typedef enum
{
	SECUREVALUE_SLOT_SD = 0x1000, ///< SD application.
} FS_SecureValueSlot;

/// Card SPI baud rate.
typedef enum
{
	BAUDRATE_512KHZ = 0, ///< 512KHz.
	BAUDRATE_1MHZ   = 1, ///< 1MHz.
	BAUDRATE_2MHZ   = 2, ///< 2MHz.
	BAUDRATE_4MHZ   = 3, ///< 4MHz.
	BAUDRATE_8MHZ   = 4, ///< 8MHz.
	BAUDRATE_16MHZ  = 5, ///< 16MHz.
} FS_CardSpiBaudRate;

/// Card SPI bus mode.
typedef enum
{
	BUSMODE_1BIT = 0, ///< 1-bit.
	BUSMODE_4BIT = 1, ///< 4-bit.
} FS_CardSpiBusMode;

/// Card SPI bus mode.
typedef enum
{
	SPECIALCONTENT_UPDATE    = 1, ///< Update.
	SPECIALCONTENT_MANUAL    = 2, ///< Manual.
	SPECIALCONTENT_DLP_CHILD = 3, ///< DLP child.
} FS_SpecialContentType;

typedef enum
{
	CARD_CTR = 0, ///< CTR card.
	CARD_TWL = 1, ///< TWL card.
} FS_CardType;

/// FS control actions.
typedef enum
{
	FS_ACTION_UNKNOWN = 0,
} FS_Action;

/// Archive control actions.
typedef enum
{
	ARCHIVE_ACTION_COMMIT_SAVE_DATA = 0, ///< Commits save data changes. No inputs/outputs.
	ARCHIVE_ACTION_GET_TIMESTAMP    = 1, ///< Retrieves a file's last-modified timestamp. In: "u16*, UTF-16 Path", Out: "u64, Time Stamp".
	ARCHIVE_ACTION_UNKNOWN          = 0x789D, //< Unknown action; calls FSPXI command 0x56. In: "FS_Path instance", Out: "u32[4], Unknown"
} FS_ArchiveAction;

/// Secure save control actions.
typedef enum
{
	SECURESAVE_ACTION_DELETE = 0, ///< Deletes a save's secure value. In: "u64, ((SecureValueSlot << 32) | (TitleUniqueId << 8) | TitleVariation)", Out: "u8, Value Existed"
	SECURESAVE_ACTION_FORMAT = 1, ///< Formats a save. No inputs/outputs.
} FS_SecureSaveAction;

/// File control actions.
typedef enum
{
	FILE_ACTION_UNKNOWN = 0,
} FS_FileAction;

/// Directory control actions.
typedef enum
{
	DIRECTORY_ACTION_UNKNOWN = 0,
} FS_DirectoryAction;

/// Directory entry.
typedef struct
{
	u16 name[0x106];      ///< UTF-16 directory name.
	char shortName[0x0A]; ///< File name.
	char shortExt[0x04];  ///< File extension.
	u8 valid;             ///< Valid flag. (Always 1)
	u8 reserved;          ///< Reserved.
	u32 attributes;       ///< Attributes.
	u64 fileSize;         ///< File size.
} FS_DirectoryEntry;

/// Archive resource information.
typedef struct
{
	u32 sectorSize;    ///< Size of each sector, in bytes.
	u32 clusterSize;   ///< Size of each cluster, in bytes.
	u32 totalClusters; ///< Total number of clusters.
	u32 freeClusters;  ///< Number of free clusters.
} FS_ArchiveResource;

/// Program information.
typedef struct
{
	u64 programId;              ///< Program ID.
	FS_MediaType mediaType : 8; ///< Media type.
	u8 padding[7];              ///< Padding.
} FS_ProgramInfo;

/// Product information.
typedef struct
{
	char productCode[0x10]; ///< Product code.
	char companyCode[0x2];  ///< Company code.
	u16 remasterVersion;    ///< Remaster version.
} FS_ProductInfo;

/// Integrity verification seed.
typedef struct
{
	u8 aesCbcMac[0x10];   ///< AES-CBC MAC over a SHA256 hash, which hashes the first 0x110-bytes of the cleartext SEED.
	u8 movableSed[0x120]; ///< The "nand/private/movable.sed", encrypted with AES-CTR using the above MAC for the counter.
} FS_IntegrityVerificationSeed;

/// Ext save data information.
typedef struct PACKED
{
	FS_MediaType mediaType : 8; ///< Media type.
	u8 unknown;                 ///< Unknown.
	u16 reserved1;              ///< Reserved.
	u64 saveId;                 ///< Save ID.
	u32 reserved2;              ///< Reserved.
} FS_ExtSaveDataInfo;

/// System save data information.
typedef struct
{
	FS_MediaType mediaType : 8; ///< Media type.
	u8 unknown;                 ///< Unknown.
	u16 reserved;               ///< Reserved.
	u32 saveId;                 ///< Save ID.
} FS_SystemSaveDataInfo;

/// Device move context.
typedef struct
{
	u8 ivs[0x10];              ///< IVs.
	u8 encryptParameter[0x10]; ///< Encrypt parameter.
} FS_DeviceMoveContext;

/// Filesystem path data, detailing the specific target of an operation.
typedef struct
{
	FS_PathType type; ///< FS path type.
	u32 size;         ///< FS path size.
	const void* data; ///< Pointer to FS path data.
} FS_Path;

/// Filesystem archive handle, providing access to a filesystem's contents.
typedef u64 FS_Archive;

/// Initializes FS.
Result fsInit(void);

/// Exits FS.
void fsExit(void);

/**
 * @brief Sets the FSUSER session to use in the current thread.
 * @param session The handle of the FSUSER session to use.
 */
void fsUseSession(Handle session);

/// Disables the FSUSER session override in the current thread.
void fsEndUseSession(void);

/**
 * @brief Exempts an archive from using alternate FS session handles provided with @ref fsUseSession
 * Instead, the archive will use the default FS session handle, opened with @ref srvGetSessionHandle
 * @param archive Archive to exempt.
 */
void fsExemptFromSession(FS_Archive archive);

/**
 * @brief Unexempts an archive from using alternate FS session handles provided with @ref fsUseSession
 * @param archive Archive to remove from the exemption list.
 */
void fsUnexemptFromSession(FS_Archive archive);

/**
 * @brief Creates an FS_Path instance.
 * @param type Type of path.
 * @param path Path to use.
 * @return The created FS_Path instance.
 */
FS_Path fsMakePath(FS_PathType type, const void* path);

/**
 * @brief Gets the current FS session handle.
 * @return The current FS session handle.
 */
Handle* fsGetSessionHandle(void);

/**
 * @brief Performs a control operation on the filesystem.
 * @param action Action to perform.
 * @param input Buffer to read input from.
 * @param inputSize Size of the input.
 * @param output Buffer to write output to.
 * @param outputSize Size of the output.
 */
Result FSUSER_Control(FS_Action action, void* input, u32 inputSize, void* output, u32 outputSize);

/**
 * @brief Initializes a FSUSER session.
 * @param session The handle of the FSUSER session to initialize.
 */
Result FSUSER_Initialize(Handle session);

/**
 * @brief Opens a file.
 * @param out Pointer to output the file handle to.
 * @param archive Archive containing the file.
 * @param path Path of the file.
 * @param openFlags Flags to open the file with.
 * @param attributes Attributes of the file.
 */
Result FSUSER_OpenFile(Handle* out, FS_Archive archive, FS_Path path, u32 openFlags, u32 attributes);

/**
 * @brief Opens a file directly, bypassing the requirement of an opened archive handle.
 * @param out Pointer to output the file handle to.
 * @param archiveId ID of the archive containing the file.
 * @param archivePath Path of the archive containing the file.
 * @param filePath Path of the file.
 * @param openFlags Flags to open the file with.
 * @param attributes Attributes of the file.
 */
Result FSUSER_OpenFileDirectly(Handle* out, FS_ArchiveID archiveId, FS_Path archivePath, FS_Path filePath, u32 openFlags, u32 attributes);

/**
 * @brief Deletes a file.
 * @param archive Archive containing the file.
 * @param path Path of the file.
 */
Result FSUSER_DeleteFile(FS_Archive archive, FS_Path path);

/**
 * @brief Renames a file.
 * @param srcArchive Archive containing the source file.
 * @param srcPath Path of the source file.
 * @param dstArchive Archive containing the destination file.
 * @param dstPath Path of the destination file.
 */
Result FSUSER_RenameFile(FS_Archive srcArchive, FS_Path srcPath, FS_Archive dstArchive, FS_Path dstPath);

/**
 * @brief Deletes a directory, failing if it is not empty.
 * @param archive Archive containing the directory.
 * @param path Path of the directory.
 */
Result FSUSER_DeleteDirectory(FS_Archive archive, FS_Path path);

/**
 * @brief Deletes a directory, also deleting its contents.
 * @param archive Archive containing the directory.
 * @param path Path of the directory.
 */
Result FSUSER_DeleteDirectoryRecursively(FS_Archive archive, FS_Path path);

/**
 * @brief Creates a file.
 * @param archive Archive to create the file in.
 * @param path Path of the file.
 * @param attributes Attributes of the file.
 * @param fileSize Size of the file.
 */
Result FSUSER_CreateFile(FS_Archive archive, FS_Path path, u32 attributes, u64 fileSize);

/**
 * @brief Creates a directory
 * @param archive Archive to create the directory in.
 * @param path Path of the directory.
 * @param attributes Attributes of the directory.
 */
Result FSUSER_CreateDirectory(FS_Archive archive, FS_Path path, u32 attributes);

/**
 * @brief Renames a directory.
 * @param srcArchive Archive containing the source directory.
 * @param srcPath Path of the source directory.
 * @param dstArchive Archive containing the destination directory.
 * @param dstPath Path of the destination directory.
 */
Result FSUSER_RenameDirectory(FS_Archive srcArchive, FS_Path srcPath, FS_Archive dstArchive, FS_Path dstPath);

/**
 * @brief Opens a directory.
 * @param out Pointer to output the directory handle to.
 * @param archive Archive containing the directory.
 * @param path Path of the directory.
 */
Result FSUSER_OpenDirectory(Handle *out, FS_Archive archive, FS_Path path);

/**
 * @brief Opens an archive.
 * @param archive Pointer to output the opened archive to.
 * @param id ID of the archive.
 * @param path Path of the archive.
 */
Result FSUSER_OpenArchive(FS_Archive* archive, FS_ArchiveID id, FS_Path path);

/**
 * @brief Performs a control operation on an archive.
 * @param archive Archive to control.
 * @param action Action to perform.
 * @param input Buffer to read input from.
 * @param inputSize Size of the input.
 * @param output Buffer to write output to.
 * @param outputSize Size of the output.
 */
Result FSUSER_ControlArchive(FS_Archive archive, FS_ArchiveAction action, void* input, u32 inputSize, void* output, u32 outputSize);

/**
 * @brief Closes an archive.
 * @param archive Archive to close.
 */
Result FSUSER_CloseArchive(FS_Archive archive);

/**
 * @brief Gets the number of free bytes within an archive.
 * @param freeBytes Pointer to output the free bytes to.
 * @param archive Archive to check.
 */
Result FSUSER_GetFreeBytes(u64* freeBytes, FS_Archive archive);

/**
 * @brief Gets the inserted card type.
 * @param type Pointer to output the card type to.
 */
Result FSUSER_GetCardType(FS_CardType* type);

/**
 * @brief Gets the SDMC archive resource information.
 * @param archiveResource Pointer to output the archive resource information to.
 */
Result FSUSER_GetSdmcArchiveResource(FS_ArchiveResource* archiveResource);

/**
 * @brief Gets the NAND archive resource information.
 * @param archiveResource Pointer to output the archive resource information to.
 */
Result FSUSER_GetNandArchiveResource(FS_ArchiveResource* archiveResource);

/**
 * @brief Gets the last SDMC fatfs error.
 * @param error Pointer to output the error to.
 */
Result FSUSER_GetSdmcFatfsError(u32* error);

/**
 * @brief Gets whether an SD card is detected.
 * @param detected Pointer to output the detection status to.
 */
Result FSUSER_IsSdmcDetected(bool *detected);

/**
 * @brief Gets whether the SD card is writable.
 * @param writable Pointer to output the writable status to.
 */
Result FSUSER_IsSdmcWritable(bool *writable);

/**
 * @brief Gets the SDMC CID.
 * @param out Pointer to output the CID to.
 * @param length Length of the CID buffer. (should be 0x10)
 */
Result FSUSER_GetSdmcCid(u8* out, u32 length);

/**
 * @brief Gets the NAND CID.
 * @param out Pointer to output the CID to.
 * @param length Length of the CID buffer. (should be 0x10)
 */
Result FSUSER_GetNandCid(u8* out, u32 length);

/**
 * @brief Gets the SDMC speed info.
 * @param speedInfo Pointer to output the speed info to.
 */
Result FSUSER_GetSdmcSpeedInfo(u32 *speedInfo);

/**
 * @brief Gets the NAND speed info.
 * @param speedInfo Pointer to output the speed info to.
 */
Result FSUSER_GetNandSpeedInfo(u32 *speedInfo);

/**
 * @brief Gets the SDMC log.
 * @param out Pointer to output the log to.
 * @param length Length of the log buffer.
 */
Result FSUSER_GetSdmcLog(u8* out, u32 length);

/**
 * @brief Gets the NAND log.
 * @param out Pointer to output the log to.
 * @param length Length of the log buffer.
 */
Result FSUSER_GetNandLog(u8* out, u32 length);

/// Clears the SDMC log.
Result FSUSER_ClearSdmcLog(void);

/// Clears the NAND log.
Result FSUSER_ClearNandLog(void);

/**
 * @brief Gets whether a card is inserted.
 * @param inserted Pointer to output the insertion status to.
 */
Result FSUSER_CardSlotIsInserted(bool* inserted);

/**
 * @brief Powers on the card slot.
 * @param status Pointer to output the power status to.
 */
Result FSUSER_CardSlotPowerOn(bool* status);

/**
 * @brief Powers off the card slot.
 * @param status Pointer to output the power status to.
 */
Result FSUSER_CardSlotPowerOff(bool* status);

/**
 * @brief Gets the card's power status.
 * @param status Pointer to output the power status to.
 */
Result FSUSER_CardSlotGetCardIFPowerStatus(bool* status);

/**
 * @brief Executes a CARDNOR direct command.
 * @param commandId ID of the command.
 */
Result FSUSER_CardNorDirectCommand(u8 commandId);

/**
 * @brief Executes a CARDNOR direct command with an address.
 * @param commandId ID of the command.
 * @param address Address to provide.
 */
Result FSUSER_CardNorDirectCommandWithAddress(u8 commandId, u32 address);

/**
 * @brief Executes a CARDNOR direct read.
 * @param commandId ID of the command.
 * @param size Size of the output buffer.
 * @param output Output buffer.
 */
Result FSUSER_CardNorDirectRead(u8 commandId, u32 size, void* output);

/**
 * @brief Executes a CARDNOR direct read with an address.
 * @param commandId ID of the command.
 * @param address Address to provide.
 * @param size Size of the output buffer.
 * @param output Output buffer.
 */
Result FSUSER_CardNorDirectReadWithAddress(u8 commandId, u32 address, u32 size, void* output);

/**
 * @brief Executes a CARDNOR direct write.
 * @param commandId ID of the command.
 * @param size Size of the input buffer.
 * @param output Input buffer.
 */
Result FSUSER_CardNorDirectWrite(u8 commandId, u32 size, const void* input);

/**
 * @brief Executes a CARDNOR direct write with an address.
 * @param commandId ID of the command.
 * @param address Address to provide.
 * @param size Size of the input buffer.
 * @param input Input buffer.
 */
Result FSUSER_CardNorDirectWriteWithAddress(u8 commandId, u32 address, u32 size, const void* input);

/**
 * @brief Executes a CARDNOR 4xIO direct read.
 * @param commandId ID of the command.
 * @param address Address to provide.
 * @param size Size of the output buffer.
 * @param output Output buffer.
 */
Result FSUSER_CardNorDirectRead_4xIO(u8 commandId, u32 address, u32 size, void* output);

/**
 * @brief Executes a CARDNOR direct CPU write without verify.
 * @param address Address to provide.
 * @param size Size of the input buffer.
 * @param output Input buffer.
 */
Result FSUSER_CardNorDirectCpuWriteWithoutVerify(u32 address, u32 size, const void* input);

/**
 * @brief Executes a CARDNOR direct sector erase without verify.
 * @param address Address to provide.
 */
Result FSUSER_CardNorDirectSectorEraseWithoutVerify(u32 address);

/**
 * @brief Gets a process's product info.
 * @param info Pointer to output the product info to.
 * @param processId ID of the process.
 */
Result FSUSER_GetProductInfo(FS_ProductInfo* info, u32 processId);

/**
 * @brief Gets a process's program launch info.
 * @param info Pointer to output the program launch info to.
 * @param processId ID of the process.
 */
Result FSUSER_GetProgramLaunchInfo(FS_ProgramInfo* info, u32 processId);

/**
 * @brief Sets the CARDSPI baud rate.
 * @param baudRate Baud rate to set.
 */
Result FSUSER_SetCardSpiBaudRate(FS_CardSpiBaudRate baudRate);

/**
 * @brief Sets the CARDSPI bus mode.
 * @param busMode Bus mode to set.
 */
Result FSUSER_SetCardSpiBusMode(FS_CardSpiBusMode busMode);

/// Sends initialization info to ARM9.
Result FSUSER_SendInitializeInfoTo9(void);

/**
 * @brief Gets a special content's index.
 * @param index Pointer to output the index to.
 * @param mediaType Media type of the special content.
 * @param programId Program ID owning the special content.
 * @param type Type of special content.
 */
Result FSUSER_GetSpecialContentIndex(u16* index, FS_MediaType mediaType, u64 programId, FS_SpecialContentType type);

/**
 * @brief Gets the legacy ROM header of a program.
 * @param mediaType Media type of the program.
 * @param programId ID of the program.
 * @param header Pointer to output the legacy ROM header to. (size = 0x3B4)
 */
Result FSUSER_GetLegacyRomHeader(FS_MediaType mediaType, u64 programId, void* header);

/**
 * @brief Gets the legacy banner data of a program.
 * @param mediaType Media type of the program.
 * @param programId ID of the program.
 * @param header Pointer to output the legacy banner data to. (size = 0x23C0)
 */
Result FSUSER_GetLegacyBannerData(FS_MediaType mediaType, u64 programId, void* banner);

/**
 * @brief Checks a process's authority to access a save data archive.
 * @param access Pointer to output the access status to.
 * @param mediaType Media type of the save data.
 * @param saveId ID of the save data.
 * @param processId ID of the process to check.
 */
Result FSUSER_CheckAuthorityToAccessExtSaveData(bool* access, FS_MediaType mediaType, u64 saveId, u32 processId);

/**
 * @brief Queries the total quota size of a save data archive.
 * @param quotaSize Pointer to output the quota size to.
 * @param directories Number of directories.
 * @param files Number of files.
 * @param fileSizeCount Number of file sizes to provide.
 * @param fileSizes File sizes to provide.
 */
Result FSUSER_QueryTotalQuotaSize(u64* quotaSize, u32 directories, u32 files, u32 fileSizeCount, u64* fileSizes);

/**
 * @brief Abnegates an access right.
 * @param accessRight Access right to abnegate.
 */
Result FSUSER_AbnegateAccessRight(u32 accessRight);

/// Deletes the 3DS SDMC root.
Result FSUSER_DeleteSdmcRoot(void);

/// Deletes all ext save data on the NAND.
Result FSUSER_DeleteAllExtSaveDataOnNand(void);

/// Initializes the CTR file system.
Result FSUSER_InitializeCtrFileSystem(void);

/// Creates the FS seed.
Result FSUSER_CreateSeed(void);

/**
 * @brief Retrieves archive format info.
 * @param totalSize Pointer to output the total size to.
 * @param directories Pointer to output the number of directories to.
 * @param files Pointer to output the number of files to.
 * @param duplicateData Pointer to output whether to duplicate data to.
 * @param archiveId ID of the archive.
 * @param path Path of the archive.
 */
Result FSUSER_GetFormatInfo(u32* totalSize, u32* directories, u32* files, bool* duplicateData, FS_ArchiveID archiveId, FS_Path path);

/**
 * @brief Gets the legacy ROM header of a program.
 * @param headerSize Size of the ROM header.
 * @param mediaType Media type of the program.
 * @param programId ID of the program.
 * @param header Pointer to output the legacy ROM header to.
 */
Result FSUSER_GetLegacyRomHeader2(u32 headerSize, FS_MediaType mediaType, u64 programId, void* header);

/**
 * @brief Gets the CTR SDMC root path.
 * @param out Pointer to output the root path to.
 * @param length Length of the output buffer.
 */
Result FSUSER_GetSdmcCtrRootPath(u8* out, u32 length);

/**
 * @brief Gets an archive's resource information.
 * @param archiveResource Pointer to output the archive resource information to.
 * @param mediaType System media type to check.
 */
Result FSUSER_GetArchiveResource(FS_ArchiveResource* archiveResource, FS_SystemMediaType mediaType);

/**
 * @brief Exports the integrity verification seed.
 * @param seed Pointer to output the seed to.
 */
Result FSUSER_ExportIntegrityVerificationSeed(FS_IntegrityVerificationSeed* seed);

/**
 * @brief Imports an integrity verification seed.
 * @param seed Seed to import.
 */
Result FSUSER_ImportIntegrityVerificationSeed(FS_IntegrityVerificationSeed* seed);

/**
 * @brief Formats save data.
 * @param archiveId ID of the save data archive.
 * @param path Path of the save data.
 * @param blocks Size of the save data in blocks. (512 bytes)
 * @param directories Number of directories.
 * @param files Number of files.
 * @param directoryBuckets Directory hash tree bucket count.
 * @param fileBuckets File hash tree bucket count.
 * @param duplicateData Whether to store an internal duplicate of the data.
 */
Result FSUSER_FormatSaveData(FS_ArchiveID archiveId, FS_Path path, u32 blocks, u32 directories, u32 files, u32 directoryBuckets, u32 fileBuckets, bool duplicateData);

/**
 * @brief Gets the legacy sub banner data of a program.
 * @param bannerSize Size of the banner.
 * @param mediaType Media type of the program.
 * @param programId ID of the program.
 * @param header Pointer to output the legacy sub banner data to.
 */
Result FSUSER_GetLegacySubBannerData(u32 bannerSize, FS_MediaType mediaType, u64 programId, void* banner);

/**
 * @brief Hashes the given data and outputs a SHA256 hash.
 * @param data Pointer to the data to be hashed.
 * @param inputSize The size of the data.
 * @param hash Hash output pointer.
 */
Result FSUSER_UpdateSha256Context(const void* data, u32 inputSize, u8* hash);

/**
 * @brief Reads from a special file.
 * @param bytesRead Pointer to output the number of bytes read to.
 * @param fileOffset Offset of the file.
 * @param size Size of the buffer.
 * @param data Buffer to read to.
 */
Result FSUSER_ReadSpecialFile(u32* bytesRead, u64 fileOffset, u32 size, void* data);

/**
 * @brief Gets the size of a special file.
 * @param fileSize Pointer to output the size to.
 */
Result FSUSER_GetSpecialFileSize(u64* fileSize);

/**
 * @brief Creates ext save data.
 * @param info Info of the save data.
 * @param directories Number of directories.
 * @param files Number of files.
 * @param sizeLimit Size limit of the save data.
 * @param smdhSize Size of the save data's SMDH data.
 * @param smdh SMDH data.
 */
Result FSUSER_CreateExtSaveData(FS_ExtSaveDataInfo info, u32 directories, u32 files, u64 sizeLimit, u32 smdhSize, u8* smdh);

/**
 * @brief Deletes ext save data.
 * @param info Info of the save data.
 */
Result FSUSER_DeleteExtSaveData(FS_ExtSaveDataInfo info);

/**
 * @brief Reads the SMDH icon of ext save data.
 * @param bytesRead Pointer to output the number of bytes read to.
 * @param info Info of the save data.
 * @param smdhSize Size of the save data SMDH.
 * @param smdh Pointer to output SMDH data to.
 */
Result FSUSER_ReadExtSaveDataIcon(u32* bytesRead, FS_ExtSaveDataInfo info, u32 smdhSize, u8* smdh);

/**
 * @brief Gets an ext data archive's block information.
 * @param totalBlocks Pointer to output the total blocks to.
 * @param freeBlocks Pointer to output the free blocks to.
 * @param blockSize Pointer to output the block size to.
 * @param info Info of the save data.
 */
Result FSUSER_GetExtDataBlockSize(u64* totalBlocks, u64* freeBlocks, u32* blockSize, FS_ExtSaveDataInfo info);

/**
 * @brief Enumerates ext save data.
 * @param idsWritten Pointer to output the number of IDs written to.
 * @param idsSize Size of the IDs buffer.
 * @param mediaType Media type to enumerate over.
 * @param idSize Size of each ID element.
 * @param shared Whether to enumerate shared ext save data.
 * @param ids Pointer to output IDs to.
 */
Result FSUSER_EnumerateExtSaveData(u32* idsWritten, u32 idsSize, FS_MediaType mediaType, u32 idSize, bool shared, u8* ids);

/**
 * @brief Creates system save data.
 * @param info Info of the save data.
 * @param totalSize Total size of the save data.
 * @param blockSize Block size of the save data. (usually 0x1000)
 * @param directories Number of directories.
 * @param files Number of files.
 * @param directoryBuckets Directory hash tree bucket count.
 * @param fileBuckets File hash tree bucket count.
 * @param duplicateData Whether to store an internal duplicate of the data.
 */
Result FSUSER_CreateSystemSaveData(FS_SystemSaveDataInfo info, u32 totalSize, u32 blockSize, u32 directories, u32 files, u32 directoryBuckets, u32 fileBuckets, bool duplicateData);

/**
 * @brief Deletes system save data.
 * @param info Info of the save data.
 */
Result FSUSER_DeleteSystemSaveData(FS_SystemSaveDataInfo info);

/**
 * @brief Initiates a device move as the source device.
 * @param context Pointer to output the context to.
 */
Result FSUSER_StartDeviceMoveAsSource(FS_DeviceMoveContext* context);

/**
 * @brief Initiates a device move as the destination device.
 * @param context Context to use.
 * @param clear Whether to clear the device's data first.
 */
Result FSUSER_StartDeviceMoveAsDestination(FS_DeviceMoveContext context, bool clear);

/**
 * @brief Sets an archive's priority.
 * @param archive Archive to use.
 * @param priority Priority to set.
 */
Result FSUSER_SetArchivePriority(FS_Archive archive, u32 priority);

/**
 * @brief Gets an archive's priority.
 * @param priority Pointer to output the priority to.
 * @param archive Archive to use.
 */
Result FSUSER_GetArchivePriority(u32* priority, FS_Archive archive);

/**
 * @brief Configures CTRCARD latency emulation.
 * @param latency Latency to apply, in milliseconds.
 * @param emulateEndurance Whether to emulate card endurance.
 */
Result FSUSER_SetCtrCardLatencyParameter(u64 latency, bool emulateEndurance);

/**
 * @brief Toggles cleaning up invalid save data.
 * @param enable Whether to enable cleaning up invalid save data.
 */
Result FSUSER_SwitchCleanupInvalidSaveData(bool enable);

/**
 * @brief Enumerates system save data.
 * @param idsWritten Pointer to output the number of IDs written to.
 * @param idsSize Size of the IDs buffer.
 * @param ids Pointer to output IDs to.
 */
Result FSUSER_EnumerateSystemSaveData(u32* idsWritten, u32 idsSize, u32* ids);

/**
 * @brief Initializes a FSUSER session with an SDK version.
 * @param session The handle of the FSUSER session to initialize.
 * @param version SDK version to initialize with.
 */
Result FSUSER_InitializeWithSdkVersion(Handle session, u32 version);

/**
 * @brief Sets the file system priority.
 * @param priority Priority to set.
 */
Result FSUSER_SetPriority(u32 priority);

/**
 * @brief Gets the file system priority.
 * @param priority Pointer to output the priority to.
 */
Result FSUSER_GetPriority(u32* priority);

/**
 * @brief Sets the save data secure value.
 * @param value Secure value to set.
 * @param slot Slot of the secure value.
 * @param titleUniqueId Unique ID of the title. (default = 0)
 * @param titleVariation Variation of the title. (default = 0)
 */
Result FSUSER_SetSaveDataSecureValue(u64 value, FS_SecureValueSlot slot, u32 titleUniqueId, u8 titleVariation);

/**
 * @brief Gets the save data secure value.
 * @param exists Pointer to output whether the secure value exists to.
 * @param value Pointer to output the secure value to.
 * @param slot Slot of the secure value.
 * @param titleUniqueId Unique ID of the title. (default = 0)
 * @param titleVariation Variation of the title. (default = 0)
 */
Result FSUSER_GetSaveDataSecureValue(bool* exists, u64* value, FS_SecureValueSlot slot, u32 titleUniqueId, u8 titleVariation);

/**
 * @brief Performs a control operation on a secure save.
 * @param action Action to perform.
 * @param input Buffer to read input from.
 * @param inputSize Size of the input.
 * @param output Buffer to write output to.
 * @param outputSize Size of the output.
 */
Result FSUSER_ControlSecureSave(FS_SecureSaveAction action, void* input, u32 inputSize, void* output, u32 outputSize);

/**
 * @brief Gets the media type of the current application.
 * @param mediaType Pointer to output the media type to.
 */
Result FSUSER_GetMediaType(FS_MediaType* mediaType);

/**
 * @brief Performs a control operation on a file.
 * @param handle Handle of the file.
 * @param action Action to perform.
 * @param input Buffer to read input from.
 * @param inputSize Size of the input.
 * @param output Buffer to write output to.
 * @param outputSize Size of the output.
 */
Result FSFILE_Control(Handle handle, FS_FileAction action, void* input, u32 inputSize, void* output, u32 outputSize);

/**
 * @brief Opens a handle to a sub-section of a file.
 * @param handle Handle of the file.
 * @param subFile Pointer to output the sub-file to.
 * @param offset Offset of the sub-section.
 * @param size Size of the sub-section.
 */
Result FSFILE_OpenSubFile(Handle handle, Handle* subFile, u64 offset, u64 size);

/**
 * @brief Reads from a file.
 * @param handle Handle of the file.
 * @param bytesRead Pointer to output the number of bytes read to.
 * @param offset Offset to read from.
 * @param buffer Buffer to read to.
 * @param size Size of the buffer.
 */
Result FSFILE_Read(Handle handle, u32* bytesRead, u64 offset, void* buffer, u32 size);

/**
 * @brief Writes to a file.
 * @param handle Handle of the file.
 * @param bytesWritten Pointer to output the number of bytes written to.
 * @param offset Offset to write to.
 * @param buffer Buffer to write from.
 * @param size Size of the buffer.
 * @param flags Flags to use when writing.
 */
Result FSFILE_Write(Handle handle, u32* bytesWritten, u64 offset, const void* buffer, u32 size, u32 flags);

/**
 * @brief Gets the size of a file.
 * @param handle Handle of the file.
 * @param size Pointer to output the size to.
 */
Result FSFILE_GetSize(Handle handle, u64* size);

/**
 * @brief Sets the size of a file.
 * @param handle Handle of the file.
 * @param size Size to set.
 */
Result FSFILE_SetSize(Handle handle, u64 size);

/**
 * @brief Gets the attributes of a file.
 * @param handle Handle of the file.
 * @param attributes Pointer to output the attributes to.
 */
Result FSFILE_GetAttributes(Handle handle, u32* attributes);

/**
 * @brief Sets the attributes of a file.
 * @param handle Handle of the file.
 * @param attributes Attributes to set.
 */
Result FSFILE_SetAttributes(Handle handle, u32 attributes);

/**
 * @brief Closes a file.
 * @param handle Handle of the file.
 */
Result FSFILE_Close(Handle handle);

/**
 * @brief Flushes a file's contents.
 * @param handle Handle of the file.
 */
Result FSFILE_Flush(Handle handle);

/**
 * @brief Sets a file's priority.
 * @param handle Handle of the file.
 * @param priority Priority to set.
 */
Result FSFILE_SetPriority(Handle handle, u32 priority);

/**
 * @brief Gets a file's priority.
 * @param handle Handle of the file.
 * @param priority Pointer to output the priority to.
 */
Result FSFILE_GetPriority(Handle handle, u32* priority);

/**
 * @brief Opens a duplicate handle to a file.
 * @param handle Handle of the file.
 * @param linkFile Pointer to output the link handle to.
 */
Result FSFILE_OpenLinkFile(Handle handle, Handle* linkFile);

/**
 * @brief Performs a control operation on a directory.
 * @param handle Handle of the directory.
 * @param action Action to perform.
 * @param input Buffer to read input from.
 * @param inputSize Size of the input.
 * @param output Buffer to write output to.
 * @param outputSize Size of the output.
 */
Result FSDIR_Control(Handle handle, FS_DirectoryAction action, void* input, u32 inputSize, void* output, u32 outputSize);

/**
 * @brief Reads one or more directory entries.
 * @param handle Handle of the directory.
 * @param entriesRead Pointer to output the number of entries read to.
 * @param entryCount Number of entries to read.
 * @param entryOut Pointer to output directory entries to.
 */
Result FSDIR_Read(Handle handle, u32* entriesRead, u32 entryCount, FS_DirectoryEntry* entries);

/**
 * @brief Closes a directory.
 * @param handle Handle of the directory.
 */
Result FSDIR_Close(Handle handle);

/**
 * @brief Sets a directory's priority.
 * @param handle Handle of the directory.
 * @param priority Priority to set.
 */
Result FSDIR_SetPriority(Handle handle, u32 priority);

/**
 * @brief Gets a directory's priority.
 * @param handle Handle of the directory.
 * @param priority Pointer to output the priority to.
 */
Result FSDIR_GetPriority(Handle handle, u32* priority);
