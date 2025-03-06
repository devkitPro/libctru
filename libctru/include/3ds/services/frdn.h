/**
 * @file frdn.h
 * @brief Friend Network Daemon Service
 */
#pragma once
#include <3ds/types.h>

/**
 * @brief Initializes the friend network daemon service.
 */
Result frdnInit();

/// Exits the friend network daemon service.
void frdnExit(void);

/// Get the friend network daemon service handle.
Handle *frdnGetSessionHandle(void);

/**
 * @brief Gets the handle signaled whenever the friends daemon changes its status.
 * @param evt Pointer to write the shared status changed event handle to.
 */
Result FRDN_GetHandleOfNdmStatusChangedEvent(Handle *evt);

/**
 * @brief Resumes the friends daemon.
 */
Result FRDN_Resume();

/**
 * @brief Suspends the friends daemon.
 * @param immediately Whether or not to suspend immediately.
 */
Result FRDN_SuspendAsync(bool immediately);

/**
 * @brief Queries the status of the friends daemon.
 * @param status Pointer to output the status value to.
 */
Result FRDN_QueryStatus(u8 *status);