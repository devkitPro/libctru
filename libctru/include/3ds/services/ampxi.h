/**
 * @file ampxi.h
 * @brief AMPXI service. This is normally not accessible to userland apps. https://3dbrew.org/wiki/Application_Manager_Services_PXI
 */
#pragma once

/**
 * @brief Initializes AMPXI.
 * @param servhandle Optional service session handle to use for AMPXI, if zero srvGetServiceHandle() will be used.
 */
Result ampxiInit(Handle servhandle);

/// Exits AMPXI.
void ampxiExit(void);

/**
 * @brief Writes a TWL save-file to NAND. https://www.3dbrew.org/wiki/AMPXI:WriteTWLSavedata
 * @param titleid ID of the TWL title.
 * @param buffer Savedata buffer ptr.
 * @param size Size of the savedata buffer.
 * @param image_filepos Filepos to use for writing the data to the NAND savedata file.
 * @param section_type https://www.3dbrew.org/wiki/AMPXI:WriteTWLSavedata
 * @param operation https://3dbrew.org/wiki/AM:ImportDSiWare
 */
Result AMPXI_WriteTWLSavedata(u64 titleid, u8 *buffer, u32 size, u32 image_filepos, u8 section_type, u8 operation);

/**
 * @brief Finalizes title installation. https://3dbrew.org/wiki/AMPXI:InstallTitlesFinish
 * @param mediaType Mediatype of the titles to finalize.
 * @param db Which title database to use.
 * @param size Size of the savedata buffer.
 * @param titlecount Total titles.
 * @param tidlist List of titleIDs.
 */
Result AMPXI_InstallTitlesFinish(FS_MediaType mediaType, u8 db, u32 titlecount, u64 *tidlist);
