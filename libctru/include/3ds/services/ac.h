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

/**
 * @brief Gets the current Wifi status.
 * @param out Pointer to output the current Wifi status to. (1 = not connected, 3 = connected)
 */
Result ACU_GetStatus(u32 *out);

/**
 * @brief Gets the current Wifi security mode.
 * @param out Pointer to output the current Wifi security mode to. (0 = Open Authentication, 1 = WEP 40-bit, 2 = WEP 104-bit, 3 = WEP 128-bit, 4 = WPA TKIP, 5 = WPA2 TKIP, 6 = WPA AES, 7 = WPA2 AES)
 */
Result ACU_GetSecurityMode(u32 *out);

/**
 * @brief Gets the current Wifi SSID length.
 * @param out Pointer to output the current Wifi SSID length to.
 */
Result ACU_GetSsidLength(u32 *out);
