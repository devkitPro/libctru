/**
 * @file srv.h
 * @brief Service API.
 */
#pragma once

//! Initializes the service API.
Result srvInit(void);

//! Exits the service API.
Result srvExit(void);

/**
 * @brief Gets the current service API session handle.
 * @return The current service API session handle.
 */
Handle *srvGetSessionHandle(void);

//! Registers the current process as a client to the service API.
Result srvRegisterClient(void);

/**
 * @brief Retrieves a service handle, bypassing the handle list.
 * @param out Pointer to write the handle to.
 * @param name Name of the service.
 */
Result srvGetServiceHandleDirect(Handle* out, const char* name);

/**
 * @brief Retrieves a service handle.
 * @param out Pointer to write the handle to.
 * @param name Name of the service.
 */
Result srvGetServiceHandle(Handle* out, const char* name);

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

//! Initializes the srv:pm port.
Result srvPmInit(void);

/**
 * @brief Registers a process with srv:pm.
 * @param procid ID of the process to register.
 * @param count Number of services to register access to.
 * @param serviceaccesscontrol Service access permissions of the process.
 */
Result srvRegisterProcess(u32 procid, u32 count, void *serviceaccesscontrol);

/**
 * @brief Unregisters a process with srv:pm.
 * @param procid ID of the process to unregister.
 */
Result srvUnregisterProcess(u32 procid);
