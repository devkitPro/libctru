/**
 * @file archive.h
 * @brief FS_Archive driver
 */
#pragma once

#include <sys/types.h>

#include <3ds/types.h>
#include <3ds/services/fs.h>

#define ARCHIVE_DIRITER_MAGIC 0x68637261 /* "arch" */

/*! Open directory struct */
typedef struct
{
  u32               magic;          /*! "arch" */
  Handle            fd;             /*! CTRU handle */
  ssize_t           index;          /*! Current entry index */
  size_t            size;           /*! Current batch size */
  FS_DirectoryEntry entry_data[32]; /*! Temporary storage for reading entries */
} archive_dir_t;

/// Mounts the SD
Result archiveMountSdmc(void);

/// Mounts and opens an archive as deviceName
/// Returns either an archive open error code, or -1 for generic failure
Result archiveMount(FS_ArchiveID archiveID, FS_Path archivePath, const char *deviceName);

/// Uses FSUSER_ControlArchive with control action ARCHIVE_ACTION_COMMIT_SAVE_DATA on the opened archive. Not done automatically at unmount.
/// Returns -1 if the specified device is not found
Result archiveCommitSaveData(const char *deviceName);

/// Unmounts the specified device, closing its archive in the process
/// Returns -1 if the specified device was not found
Result archiveUnmount(const char *deviceName);

/// Unmounts all devices and cleans up any resources used by the driver
Result archiveUnmountAll(void);

/// Get a file's mtime
Result archive_getmtime(const char *name, u64 *mtime);
