/**
 * @file ac.h
 * @brief AC service.
 */
#pragma once

/// Initializes AC.
Result acInit(void);

/// Exits AC.
Result acExit(void);

/**
 * @brief Gets the current Wifi status.
 * @param servhandle Optional pointer to the service handle to use.
 * @param out Pointer to output the current Wifi status to. (0 = not connected, 1 = O3DS Internet, 2 = N3DS Internet)
 */
Result ACU_GetWifiStatus(Handle* servhandle, u32 *out);

/// Waits for the system to connect to the internet.
Result ACU_WaitInternetConnection(void);
