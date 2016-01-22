/**
 * @file sdmc.h
 * @brief SDMC driver.
 */
#pragma once

#include <3ds/types.h>
#include <3ds/services/fs.h>

/*! Open directory struct */
typedef struct
{
  Handle    fd;                 /*! CTRU handle */
  FS_DirectoryEntry entry_data; /*! Temporary storage for reading entries */
} sdmc_dir_t;

/// Initializes the SDMC driver.
Result sdmcInit(void);

/// Enable/disable copy in sdmc_write
void sdmcWriteSafe(bool enable);

/// Exits the SDMC driver.
Result sdmcExit(void);

/// Get a file's mtime
Result sdmc_getmtime(const char *name, u64 *mtime);
