/**
 * @file fspxi.h
 * @brief Service interface for PxiFS services. This is normally not accessible to userland apps. https://3dbrew.org/wiki/Filesystem_services_PXI
 */
#pragma once

#include <3ds/services/fs.h>
#include <3ds/types.h>

typedef u64 FSPXI_Archive;
typedef u64 FSPXI_File;
typedef u64 FSPXI_Directory;

#define DEFINE_SERVICE_METHOD(servname, funcname, ...) Result servname ## _ ## funcname(__VA_ARGS__)
#define DEFINE_PXIFS_SERVICE_METHOD(funcname, ...) DEFINE_SERVICE_METHOD(PXIFS0, funcname, __VA_ARGS__); \
                                                   DEFINE_SERVICE_METHOD(PXIFS1, funcname, __VA_ARGS__); \
                                                   DEFINE_SERVICE_METHOD(PXIFSR, funcname, __VA_ARGS__); \
                                                   DEFINE_SERVICE_METHOD(PXIFSB, funcname, __VA_ARGS__)

/**
 * @brief Initializes PxiFS0.
 * @param servhandle Optional service session handle to use for PxiFS0, if zero srvGetServiceHandle() will be used.
 */
Result pxifs0Init(Handle servhandle);

/// Exits PxiFS0.
void pxifs0Exit(void);

/**
 * @brief Initializes PxiFS1.
 * @param servhandle Optional service session handle to use for PxiFS1, if zero srvGetServiceHandle() will be used.
 */
Result pxifs1Init(Handle servhandle);

/// Exits PxiFS1.
void pxifs1Exit(void);

/**
 * @brief Initializes PxiFSR.
 * @param servhandle Optional service session handle to use for PxiFSR, if zero srvGetServiceHandle() will be used.
 */
Result pxifsRInit(Handle servhandle);

/// Exits PxiFSR.
void pxifsRExit(void);

/**
 * @brief Initializes PxiFSB.
 * @param servhandle Optional service session handle to use for PxiFSB, if zero srvGetServiceHandle() will be used.
 */
Result pxifsBInit(Handle servhandle);

/// Exits PxiFSB.
void pxifsBExit(void);

/**
 * @brief Opens a file.
 * @param out Pointer to output the file handle to.
 * @param archive Archive containing the file.
 * @param path Path of the file.
 * @param flags Flags to open the file with.
 * @param attributes Attributes of the file.
 */
DEFINE_PXIFS_SERVICE_METHOD(OpenFile, FSPXI_File* out, FSPXI_Archive archive, FS_Path path, u32 flags, u32 attributes);

/**
 * @brief Deletes a file.
 * @param archive Archive containing the file.
 * @param path Path of the file.
 */
DEFINE_PXIFS_SERVICE_METHOD(DeleteFile, FSPXI_Archive archive, FS_Path path);

/**
 * @brief Renames a file.
 * @param srcArchive Archive containing the source file.
 * @param srcPath Path of the source file.
 * @param dstArchive Archive containing the destination file.
 * @param dstPath Path of the destination file.
 */
DEFINE_PXIFS_SERVICE_METHOD(RenameFile, FSPXI_Archive srcArchive, FS_Path srcPath, FSPXI_Archive dstArchive, FS_Path dstPath);

/**
 * @brief Deletes a directory.
 * @param archive Archive containing the directory.
 * @param path Path of the directory.
 */
DEFINE_PXIFS_SERVICE_METHOD(DeleteDirectory, FSPXI_Archive archive, FS_Path path);

/**
 * @brief Creates a file.
 * @param archive Archive to create the file in.
 * @param path Path of the file.
 * @param attributes Attributes of the file.
 * @param size Size of the file.
 */
DEFINE_PXIFS_SERVICE_METHOD(CreateFile, FSPXI_Archive archive, FS_Path path, u32 attributes, u64 fileSize);

/**
 * @brief Creates a directory.
 * @param archive Archive to create the directory in.
 * @param path Path of the directory.
 * @param attributes Attributes of the directory.
 */
DEFINE_PXIFS_SERVICE_METHOD(CreateDirectory, FSPXI_Archive archive, FS_Path path, u32 attributes);

/**
 * @brief Renames a directory.
 * @param srcArchive Archive containing the source directory.
 * @param srcPath Path of the source directory.
 * @param dstArchive Archive containing the destination directory.
 * @param dstPath Path of the destination directory.
 */
DEFINE_PXIFS_SERVICE_METHOD(RenameDirectory, FSPXI_Archive srcArchive, FS_Path srcPath, FSPXI_Archive dstArchive, FS_Path dstPath);

/**
 * @brief Opens a directory.
 * @param out Pointer to output the directory handle to.
 * @param archive Archive containing the directory.
 * @param path Path of the directory.
 */
DEFINE_PXIFS_SERVICE_METHOD(OpenDirectory, FSPXI_Directory* out, FSPXI_Archive archive, FS_Path path);

/**
 * @brief Reads from a file.
 * @param file File to read from.
 * @param bytesRead Pointer to output the number of read bytes to.
 * @param offset Offset to read from.
 * @param buffer Buffer to read to.
 * @param size Size of the buffer.
 */
DEFINE_PXIFS_SERVICE_METHOD(ReadFile, FSPXI_File file, u32* bytesRead, u64 offset, void* buffer, u32 size);

/**
 * @brief Calculate SHA256 of a file.
 * @param file File to calculate the hash of.
 * @param buffer Buffer to output the hash to.
 * @param size Size of the buffer.
 */
DEFINE_PXIFS_SERVICE_METHOD(CalculateFileHashSHA256, FSPXI_File file, u32* buffer, u32 size);

/**
 * @brief Writes to a file.
 * @param file File to write to.
 * @param bytesWritten Pointer to output the number of bytes written to.
 * @param offset Offset to write to.
 * @param buffer Buffer to write from.
 * @param size Size of the buffer.
 * @param flags Flags to use when writing.
 */
DEFINE_PXIFS_SERVICE_METHOD(WriteFile, FSPXI_File file, u32* bytesWritten, u64 offset, const void* buffer, u32 size, u32 flags);

/**
 * @brief Calculates the MAC used in a DISA/DIFF header?
 * @param file Unsure
 * @param inBuffer 0x100-byte DISA/DIFF input buffer.
 * @param inSize Size of inBuffer.
 * @param outBuffer Buffer to write MAC to.
 * @param outSize Size of outBuffer.
 */
DEFINE_PXIFS_SERVICE_METHOD(CalcSavegameMAC, FSPXI_File file, void* inBuffer, u32 inSize, void* outBuffer, u32 outSize);

/**
 * @brief Get size of a file
 * @param file File to get the size of.
 * @param size Pointer to output size to.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetFileSize, FSPXI_File file, u64* size);

/**
 * @brief Set size of a file
 * @param file File to set the size of
 * @param size Size to set the file to
 */
DEFINE_PXIFS_SERVICE_METHOD(SetFileSize, FSPXI_File file, u64 size);

/**
 * @brief Close a file
 * @param file File to close
 */
DEFINE_PXIFS_SERVICE_METHOD(CloseFile, FSPXI_File file);

/**
 * @brief Reads one or more directory entries.
 * @param directory Directory to read from.
 * @param entriesRead Pointer to output the number of entries read to.
 * @param entryCount Number of entries to read.
 * @param entryOut Pointer to output directory entries to.
 */
DEFINE_PXIFS_SERVICE_METHOD(ReadDirectory, FSPXI_Directory directory, u32* entriesRead, u32 entryCount, FS_DirectoryEntry* entries);

/**
 * @brief Close a directory
 * @param directory Directory to close.
 */
DEFINE_PXIFS_SERVICE_METHOD(CloseDirectory, FSPXI_Directory directory);

/**
 * @brief Opens an archive.
 * @param archive Pointer to output the opened archive to.
 * @param id ID of the archive.
 * @param path Path of the archive.
 */
DEFINE_PXIFS_SERVICE_METHOD(OpenArchive, FSPXI_Archive* archive, FS_ArchiveID archiveID, FS_Path path);

/**
 * @brief Unknown 0x13
 */
DEFINE_PXIFS_SERVICE_METHOD(0x13, FSPXI_Archive archive, u8* out, FS_Path path);

/**
 * @brief Unknown 0x14
 */
DEFINE_PXIFS_SERVICE_METHOD(0x14, FSPXI_Archive archive, u32* out, FS_Path path);

/**
 * @brief Commits an archive's save data.
 * @param archive Archive to commit.
 * @param id Archive action sent by FSUSER_ControlArchive. Must not be 0 or 0x789D
 * @remark Unsure why id is sent. This appears to be the default action for FSUSER_ControlArchive, with every action other than 0 and 0x789D being sent to this command.
 */
DEFINE_PXIFS_SERVICE_METHOD(CommitSaveData, FSPXI_Archive archive, u32 id);

/**
 * @brief Close an archive
 * @param archive Archive to close.
 */
DEFINE_PXIFS_SERVICE_METHOD(CloseArchive, FSPXI_Archive archive);

/**
 * @brief Unknown 0x17. Appears to be an "is archive handle valid" command?
 * @param archive Archive handle to check validity of.
 * @param out Pointer to output validity to. 
 */
DEFINE_PXIFS_SERVICE_METHOD(0x17, FSPXI_Archive archive, bool* out);

/**
 * @brief Gets the inserted card type.
 * @param out Pointer to output the card type to.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetCardType, FS_CardType* out);

/**
 * @brief Gets the SDMC archive resource information.
 * @param out Pointer to output the archive resource information to.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetSdmcArchiveResource, FS_ArchiveResource* out);

/**
 * @brief Gets the NAND archive resource information.
 * @param out Pointer to output the archive resource information to.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetNandArchiveResource, FS_ArchiveResource* out);

/**
 * @brief Gets the error code from the SDMC FatFS driver
 * @param out Pointer to output the error code to
 */
DEFINE_PXIFS_SERVICE_METHOD(GetSdmcFatFsError, u32* out);

/**
 * @brief Gets whether PXIFS0 detects the SD
 * @param out Pointer to output the detection status to
 */
DEFINE_PXIFS_SERVICE_METHOD(IsSdmcDetected, bool* out);

/**
 * @brief Gets whether PXIFS0 can write to the SD
 * @param out Pointer to output the writable status to
 */
DEFINE_PXIFS_SERVICE_METHOD(IsSdmcWritable, bool* out);

/**
 * @brief Gets the SDMC CID
 * @param out Buffer to output the CID to.
 * @param size Size of buffer.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetSdmcCid, void* out, u32 size);

/**
 * @brief Gets the NAND CID
 * @param out Buffer to output the CID to.
 * @param size Size of buffer.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetNandCid, void* out, u32 size);

/**
 * @brief Gets the SDMC speed info
 * @param out Buffer to output the speed info to.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetSdmcSpeedInfo, u32* out);

/**
 * @brief Gets the NAND speed info
 * @param out Buffer to output the speed info to.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetNandSpeedInfo, u32* out);

/**
 * @brief Gets the SDMC log
 * @param out Buffer to output the log to.
 * @param size Size of buffer.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetSdmcLog, void* out, u32 size);

/**
 * @brief Gets the NAND log
 * @param out Buffer to output the log to.
 * @param size Size of buffer.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetNandLog, void* out, u32 size);

/// Clears the SDMC log
DEFINE_PXIFS_SERVICE_METHOD(ClearSdmcLog, void);

/// Clears the NAND log
DEFINE_PXIFS_SERVICE_METHOD(ClearNandLog, void);

/**
 * @brief Gets whether a card is inserted.
 * @param inserted Pointer to output the insertion status to.
 */
DEFINE_PXIFS_SERVICE_METHOD(CardSlotIsInserted, bool* inserted);

/**
 * @brief Powers on the card slot.
 * @param status Pointer to output the power status to.
 */
DEFINE_PXIFS_SERVICE_METHOD(CardSlotPowerOn, bool* status);

/**
 * @brief Powers off the card slot.
 * @param status Pointer to output the power status to.
 */
DEFINE_PXIFS_SERVICE_METHOD(CardSlotPowerOff, bool* status);

/**
 * @brief Gets the card's power status.
 * @param status Pointer to output the power status to.
 */
DEFINE_PXIFS_SERVICE_METHOD(CardSlotGetCardIFPowerStatus, bool* status);

/**
 * @brief Executes a CARDNOR direct command.
 * @param commandId ID of the command.
 */
DEFINE_PXIFS_SERVICE_METHOD(CardNorDirectCommand, u8 commandId);

/**
 * @brief Executes a CARDNOR direct command with an address.
 * @param commandId ID of the command.
 * @param address Address to provide.
 */
DEFINE_PXIFS_SERVICE_METHOD(CardNorDirectCommandWithAddress, u8 commandId, u32 address);

/**
 * @brief Executes a CARDNOR direct read.
 * @param commandId ID of the command.
 * @param size Size of the output buffer.
 * @param output Output buffer.
 */
DEFINE_PXIFS_SERVICE_METHOD(CardNorDirectRead, u8 commandId, u32 size, u8* output);

/**
 * @brief Executes a CARDNOR direct read with an address.
 * @param commandId ID of the command.
 * @param address Address to provide.
 * @param size Size of the output buffer.
 * @param output Output buffer.
 */
DEFINE_PXIFS_SERVICE_METHOD(CardNorDirectReadWithAddress, u8 commandId, u32 address, u32 size, u8* output);

/**
 * @brief Executes a CARDNOR direct write.
 * @param commandId ID of the command.
 * @param size Size of the input buffer.
 * @param output Input buffer.
 * @remark Stubbed in latest firmware, since ?.?.?
 */
DEFINE_PXIFS_SERVICE_METHOD(CardNorDirectWrite, u8 commandId, u32 size, u8* input);

/**
 * @brief Executes a CARDNOR direct write with an address.
 * @param commandId ID of the command.
 * @param address Address to provide.
 * @param size Size of the input buffer.
 * @param input Input buffer.
 */
DEFINE_PXIFS_SERVICE_METHOD(CardNorDirectWriteWithAddress, u8 commandId, u32 address, u32 size, u8* input);

/**
 * @brief Executes a CARDNOR 4xIO direct read.
 * @param commandId ID of the command.
 * @param address Address to provide.
 * @param size Size of the output buffer.
 * @param output Output buffer.
 */
DEFINE_PXIFS_SERVICE_METHOD(CardNorDirectRead_4xIO, u8 commandId, u32 address, u32 size, u8* output);

/**
 * @brief Executes a CARDNOR direct CPU write without verify.
 * @param address Address to provide.
 * @param size Size of the input buffer.
 * @param output Input buffer.
 */
DEFINE_PXIFS_SERVICE_METHOD(CardNorDirectCpuWriteWithoutVerify, u32 address, u32 size, u8* input);

/**
 * @brief Executes a CARDNOR direct sector erase without verify.
 * @param address Address to provide.
 */
DEFINE_PXIFS_SERVICE_METHOD(CardNorDirectSectorEraseWithoutVerify, u32 address);

/**
 * @brief Gets an NCCH's product info
 * @param info Pointer to output the product info to.
 * @param archive Open NCCH content archive
 */
DEFINE_PXIFS_SERVICE_METHOD(GetProductInfo, FS_ProductInfo* info, FSPXI_Archive archive);

/**
 * @brief Sets the CARDSPI baud rate.
 * @param baudRate Baud rate to set.
 */
DEFINE_PXIFS_SERVICE_METHOD(SetCardSpiBaudrate, FS_CardSpiBaudRate baudRate);

/**
 * @brief Sets the CARDSPI bus mode.
 * @param busMode Bus mode to set.
 */
DEFINE_PXIFS_SERVICE_METHOD(SetCardSpiBusMode, FS_CardSpiBusMode busMode);

/**
 * @brief Sends initialization info to ARM9
 * @param unk FS sends *(0x1FF81086)
 */
DEFINE_PXIFS_SERVICE_METHOD(SendInitializeInfoTo9, u8 unk);

/**
 * @brief Creates ext save data.
 * @param info Info of the save data.
 */
DEFINE_PXIFS_SERVICE_METHOD(CreateExtSaveData, FS_ExtSaveDataInfo info);

/**
 * @brief Deletes ext save data.
 * @param info Info of the save data.
 */
DEFINE_PXIFS_SERVICE_METHOD(DeleteExtSaveData, FS_ExtSaveDataInfo info);

/**
 * @brief Enumerates ext save data.
 * @param idsWritten Pointer to output the number of IDs written to.
 * @param idsSize Size of the IDs buffer.
 * @param mediaType Media type to enumerate over.
 * @param idSize Size of each ID element.
 * @param shared Whether to enumerate shared ext save data.
 * @param ids Pointer to output IDs to.
 */
DEFINE_PXIFS_SERVICE_METHOD(EnumerateExtSaveData, u32* idsWritten, u32 idsSize, FS_MediaType mediaType, u32 idSize, bool shared, u8* ids);

/**
 * @brief Gets a special content's index.
 * @param index Pointer to output the index to.
 * @param mediaType Media type of the special content.
 * @param programId Program ID owning the special content.
 * @param type Type of special content.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetSpecialContentIndex, u16* index, FS_MediaType mediaType, u64 programId, FS_SpecialContentType type);

/**
 * @brief Gets the legacy ROM header of a program.
 * @param mediaType Media type of the program.
 * @param programId ID of the program.
 * @param header Pointer to output the legacy ROM header to. (size = 0x3B4)
 */
DEFINE_PXIFS_SERVICE_METHOD(GetLegacyRomHeader, FS_MediaType mediaType, u64 programId, u8* header);

/**
 * @brief Gets the legacy banner data of a program.
 * @param mediaType Media type of the program.
 * @param programId ID of the program.
 * @param banner Pointer to output the legacy banner data to. (size = 0x23C0)
 * @param unk Unknown. Always 1?
 */
DEFINE_PXIFS_SERVICE_METHOD(GetLegacyBannerData, FS_MediaType mediaType, u64 programId, u8* banner, u32 unk);

/**
 * Unknown command 3D
 * @param unk Unknown. Transaction?
 */
DEFINE_PXIFS_SERVICE_METHOD(0x3D, u32 unk);

/// Deletes the 3DS SDMC root.
DEFINE_PXIFS_SERVICE_METHOD(DeleteSdmcRoot, void);

/// Deletes all ext save data on the NAND.
DEFINE_PXIFS_SERVICE_METHOD(DeleteAllExtSaveDataOnNand, void);

/// Initializes the CTR file system.
DEFINE_PXIFS_SERVICE_METHOD(InitializeCtrFilesystem, void);

/// Creates the FS seed.
DEFINE_PXIFS_SERVICE_METHOD(CreateSeed, void);

/**
 * @brief Gets the CTR SDMC root path.
 * @param out Pointer to output the root path to.
 * @param length Length of the output buffer.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetSdmcCtrRootPath, u8* out, u32 length);

/**
 * @brief Gets an archive's resource information.
 * @param archiveResource Pointer to output the archive resource information to.
 * @param mediaType System media type to check.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetArchiveResource, FS_ArchiveResource* archiveResource, FS_SystemMediaType mediaType);

/**
 * @brief Exports the integrity verification seed.
 * @param seed Pointer to output the seed to.
 */
DEFINE_PXIFS_SERVICE_METHOD(ExportIntegrityVerificationSeed, FS_IntegrityVerificationSeed* seed);

/**
 * @brief Imports an integrity verification seed.
 * @param seed Seed to import.
 */
DEFINE_PXIFS_SERVICE_METHOD(ImportIntegrityVerificationSeed, FS_IntegrityVerificationSeed* seed);

/**
 * @brief Gets the legacy sub banner data of a program.
 * @param bannerSize Size of the banner.
 * @param mediaType Media type of the program.
 * @param programId ID of the program.
 * @param header Pointer to output the legacy sub banner data to.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetLegacySubBannerData, u32 bannerSize, FS_MediaType mediaType, u64 programId, u8* banner);

/// Unknown command 47
DEFINE_PXIFS_SERVICE_METHOD(0x47, void* buf, u32 size);

/**
 * @brief Gets the last modified time of a file in an archive.
 * @param archive The archive that contains the file.
 * @param out The pointer to write the timestamp to.
 * @param path The UTF-16 path of the file.
 * @param size The size of the path.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetFileLastModified, FSPXI_Archive archive, u64* out, u16* path, u32 size);

/**
 * @brief Reads from a special file.
 * @param bytesRead Pointer to output the number of bytes read to.
 * @param fileOffset Offset of the file.
 * @param size Size of the buffer.
 * @param data Buffer to read to.
 */
DEFINE_PXIFS_SERVICE_METHOD(ReadSpecialFile, u32* bytesRead, u64 fileOffset, u32 size, u8* data);

/**
 * @brief Gets the size of a special file.
 * @param fileSize Pointer to output the size to.
 */
DEFINE_PXIFS_SERVICE_METHOD(GetSpecialFileSize, u64* fileSize);

/**
 * @brief Initiates a device move as the source device.
 * @param context Pointer to output the context to.
 */
DEFINE_PXIFS_SERVICE_METHOD(StartDeviceMoveAsSource, FS_DeviceMoveContext* context);

/**
 * @brief Initiates a device move as the destination device.
 * @param context Context to use.
 * @param clear Whether to clear the device's data first.
 */
DEFINE_PXIFS_SERVICE_METHOD(StartDeviceMoveAsDestination, FS_DeviceMoveContext context, bool clear);

/**
 * @brief Reads data and stores SHA256 hashes of blocks
 * @param file File to read from.
 * @param bytesRead Pointer to output the number of read bytes to.
 * @param offset Offset to read from.
 * @param readBuffer Pointer to store read data in.
 * @param readBufferSize Size of readBuffer.
 * @param hashtable Pointer to store SHA256 hashes in.
 * @param hashtableSize Size of hashtable.
 * @param unk Unknown. Always 0x00001000?
 */
DEFINE_PXIFS_SERVICE_METHOD(ReadFileSHA256, FSPXI_File file, u32* bytesRead, u64 offset, void* readBuffer, u32 readBufferSize, void* hashtable, u32 hashtableSize, u32 unk);

/**
 * @brief Assumedly writes data and stores SHA256 hashes of blocks
 * @param file File to write to.
 * @param bytesWritten Pointer to output the number of written bytes to.
 * @param offset Offset to write to.
 * @param writeBuffer Buffer to write from.
 * @param writeBufferSize Size of writeBuffer.
 * @param hashtable Pointer to store SHA256 hashes in.
 * @param hashtableSize Size of hashtable
 * @param unk1 Unknown. Might match with ReadFileSHA256's unknown?
 * @param unk2 Unknown.
 */
DEFINE_PXIFS_SERVICE_METHOD(WriteFileSHA256, FSPXI_File file, u32* bytesWritten, u64 offset, void* writeBuffer, u32 writeBufferSize, void* hashtable, u32 hashtableSize, u32 unk1, u32 unk2);

/// Unknown command 4F
DEFINE_PXIFS_SERVICE_METHOD(0x4F, u64 unk);

/**
 * @brief Sets the file system priority.
 * @param priority Priority to set.
 */
DEFINE_PXIFS_SERVICE_METHOD(SetPriority, u32 priority);

/**
 * @brief Toggles cleaning up invalid save data.
 * @param enable Whether to enable cleaning up invalid save data.
 */
DEFINE_PXIFS_SERVICE_METHOD(SwitchCleanupInvalidSaveData, bool enable);

/**
 * @brief Enumerates system save data.
 * @param idsWritten Pointer to output the number of IDs written to.
 * @param idsSize Size of the IDs buffer.
 * @param ids Pointer to output IDs to.
 */
DEFINE_PXIFS_SERVICE_METHOD(EnumerateSystemSaveData, u32* idsWritten, u32 idsSize, u32* ids);

/**
 * @brief Reads the NAND report.
 * @param unk Unknown
 * @param buffer Buffer to write the report to.
 * @param size Size of buffer
 */
DEFINE_PXIFS_SERVICE_METHOD(ReadNandReport, void* buffer, u32 size, u32 unk);

/**
 * @brief Unknown command 0x56
 * @remark Called by FSUSER_ControlArchive with ArchiveAction 0x789D
 */
DEFINE_PXIFS_SERVICE_METHOD(0x56, u32 (*out)[4], FS_Archive archive, FS_Path path);

#undef DEFINE_PXIFS_SERVICE_METHOD
#undef DEFINE_SERVICE_METHOD
