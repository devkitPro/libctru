/**
 * @file ac.h
 * @brief AC service.
 */
#pragma once

/// Initializes AC.
Result acInit(void);

/// Exits AC.
void acExit(void);

/// Waits for the system to connect to the internet.
Result acWaitInternetConnection(void);

/**
 * @brief Gets the current Wifi status.
 * @param out Pointer to output the current Wifi status to. (0 = not connected, 1 = O3DS Internet, 2 = N3DS Internet)
 */
Result ACU_GetWifiStatus(u32 *out);
