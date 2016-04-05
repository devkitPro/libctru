/**
 * @file sdmc.h
 * @brief SDMC driver.
 */
#pragma once

#include <sys/types.h>

#include <3ds/types.h>
#include <3ds/services/fs.h>

#define SDMC_DIRITER_MAGIC 0x73646D63 /* "sdmc" */

/*! Open directory struct */
typedef struct
{
  u32               magic;          /*! "sdmc" */
  Handle            fd;             /*! CTRU handle */
  ssize_t           index;          /*! Current entry index */
  size_t            size;           /*! Current batch size */
  FS_DirectoryEntry entry_data[32]; /*! Temporary storage for reading entries */
} sdmc_dir_t;

/// Initializes the SDMC driver.
Result sdmcInit(void);

/// Enable/disable copy in sdmc_write
void sdmcWriteSafe(bool enable);

/// Exits the SDMC driver.
Result sdmcExit(void);

/// Get a file's mtime
Result sdmc_getmtime(const char *name, u64 *mtime);
