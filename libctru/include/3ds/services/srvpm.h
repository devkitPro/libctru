/**
 * @file srvpm.h
 * @brief srv:pm service.
 */
#pragma once

/// Initializes srv:pm and the service API.
Result srvPmInit(void);

/// Exits srv:pm and the service API.
void srvPmExit(void);

/**
 * @brief Gets the current srv:pm session handle.
 * @return The current srv:pm session handle.
 */
Handle *srvPmGetSessionHandle(void);

/**
 * @brief Publishes a notification to a process.
 * @param notificationId ID of the notification.
 * @param process Process to publish to.
 */
Result SRVPM_PublishToProcess(u32 notificationId, Handle process);

/**
 * @brief Publishes a notification to all processes.
 * @param notificationId ID of the notification.
 */
Result SRVPM_PublishToAll(u32 notificationId);

/**
 * @brief Registers a process with SRV.
 * @param pid ID of the process.
 * @param count Number of services within the service access control data.
 * @param serviceAccessControlList Service Access Control list.
 */
Result SRVPM_RegisterProcess(u32 pid, u32 count, const char (*serviceAccessControlList)[8]);

/**
 * @brief Unregisters a process with SRV.
 * @param pid ID of the process.
 */
Result SRVPM_UnregisterProcess(u32 pid);

