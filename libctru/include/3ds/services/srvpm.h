/**
 * @file srvpm.h
 * @brief srv:pm service.
 */
#pragma once

/// Initializes srv:pm.
Result srvPmInit(void);

/// Exits srv:pm.
void srvPmExit(void);

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
 * @param procid ID of the process.
 * @param count Number of services within the service access control data.
 * @param serviceaccesscontrol Service Access Control list.
 */
Result SRVPM_RegisterProcess(u32 procid, u32 count, void* serviceaccesscontrol);

/**
 * @brief Unregisters a process with SRV.
 * @param procid ID of the process.
 */
Result SRVPM_UnregisterProcess(u32 procid);

