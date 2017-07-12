/**
 * @file srv.h
 * @brief Service API.
 */
#pragma once

/// Initializes the service API.
Result srvInit(void);

/// Exits the service API.
void srvExit(void);

/**
 * @brief Gets the current service API session handle.
 * @return The current service API session handle.
 */
Handle *srvGetSessionHandle(void);

/**
 * @brief Retrieves a service handle, retrieving from the environment handle list if possible.
 * @param out Pointer to write the handle to.
 * @param name Name of the service.
 */
Result srvGetServiceHandle(Handle* out, const char* name);

/// Registers the current process as a client to the service API.
Result srvRegisterClient(void);

/**
 * @brief Enables service notificatios, returning a notification semaphore.
 * @param semaphoreOut Pointer to output the notification semaphore to.
 */
Result srvEnableNotification(Handle* semaphoreOut);

/**
 * @brief Registers the current process as a service.
 * @param out Pointer to write the service handle to.
 * @param name Name of the service.
 * @param maxSessions Maximum number of sessions the service can handle.
 */
Result srvRegisterService(Handle* out, const char* name, int maxSessions);

/**
 * @brief Unregisters the current process as a service.
 * @param name Name of the service.
 */
Result srvUnregisterService(const char* name);

/**
 * @brief Retrieves a service handle.
 * @param out Pointer to output the handle to.
 * @param name Name of the service.
 */
Result srvGetServiceHandleDirect(Handle* out, const char* name);

/**
 * @brief Registers a port.
 * @param name Name of the port.
 * @param clientHandle Client handle of the port.
 */
Result srvRegisterPort(const char* name, Handle clientHandle);

/**
 * @brief Unregisters a port.
 * @param name Name of the port.
 */
Result srvUnregisterPort(const char* name);

/**
 * @brief Retrieves a port handle.
 * @param out Pointer to output the handle to.
 * @param name Name of the port.
 */
Result srvGetPort(Handle* out, const char* name);

/**
 * @brief Subscribes to a notification.
 * @param notificationId ID of the notification.
 */
Result srvSubscribe(u32 notificationId);

/**
 * @brief Unsubscribes from a notification.
 * @param notificationId ID of the notification.
 */
Result srvUnsubscribe(u32 notificationId);

/**
 * @brief Receives a notification.
 * @param notificationIdOut Pointer to output the ID of the received notification to.
 */
Result srvReceiveNotification(u32* notificationIdOut);

/**
 * @brief Publishes a notification to subscribers.
 * @param notificationId ID of the notification.
 * @param flags Flags to publish with. (bit 0 = only fire if not fired, bit 1 = do not report an error if there are more than 16 pending notifications)
 */
Result srvPublishToSubscriber(u32 notificationId, u32 flags);

/**
 * @brief Publishes a notification to subscribers and retrieves a list of all processes that were notified.
 * @param processIdCountOut Pointer to output the number of process IDs to.
 * @param processIdsOut Pointer to output the process IDs to. Should have size "60 * sizeof(u32)".
 * @param notificationId ID of the notification.
 */
Result srvPublishAndGetSubscriber(u32* processIdCountOut, u32* processIdsOut, u32 notificationId);

/**
 * @brief Checks whether a service is registered.
 * @param registeredOut Pointer to output the registration status to.
 * @param name Name of the service to check.
 */
Result srvIsServiceRegistered(bool* registeredOut, const char* name);
