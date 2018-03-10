/**
 * @file ac.h
 * @brief AC service.
 */
#pragma once

/// Wifi security modes.
typedef enum {
    AC_OPEN = 0,       ///< Open authentication.
    AC_WEP_40BIT = 1,  ///< WEP 40-bit authentication.
    AC_WEP_104BIT = 2, ///< WEP 104-bit authentication.
    AC_WEP_128BIT = 3, ///< WEP 128-bit authentication.
    AC_WPA_TKIP = 4,   ///< WPA TKIP authentication.
    AC_WPA2_TKIP = 5,  ///< WPA2 TKIP authentication.
    AC_WPA_AES = 6,    ///< WPA AES authentication.
    AC_WPA2_AES = 7,   ///< WPA2 AES authentication.
} acSecurityMode;

/// Initializes AC.
Result acInit(void);

/// Exits AC.
void acExit(void);

/// Waits for the system to connect to the internet.
Result acWaitInternetConnection(void);

/**
 * @brief Gets the connected Wifi status.
 * @param out Pointer to output the connected Wifi status to. (0 = not connected, 1 = O3DS Internet, 2 = N3DS Internet)
 */
Result ACU_GetWifiStatus(u32 *out);

/**
 * @brief Gets the connected Wifi status.
 * @param out Pointer to output the connected Wifi status to. (1 = not connected, 3 = connected)
 */
Result ACU_GetStatus(u32 *out);

/**
 * @brief Gets the connected Wifi security mode.
 * @param mode Pointer to output the connected Wifi security mode to. (0 = Open Authentication, 1 = WEP 40-bit, 2 = WEP 104-bit, 3 = WEP 128-bit, 4 = WPA TKIP, 5 = WPA2 TKIP, 6 = WPA AES, 7 = WPA2 AES)
 */
Result ACU_GetSecurityMode(acSecurityMode *mode);

/**
 * @brief Gets the connected Wifi SSID.
 * @param SSID Pointer to output the connected Wifi SSID to.
 */
Result ACU_GetSSID(char *SSID);

/**
 * @brief Gets the connected Wifi SSID length.
 * @param out Pointer to output the connected Wifi SSID length to.
 */
Result ACU_GetSSIDLength(u32 *out);

/**
 * @brief Determines whether proxy is enabled for the connected network.
 * @param enable Pointer to output the proxy status to.
 */
Result ACU_GetProxyEnable(bool *enable);

/**
 * @brief Gets the connected network's proxy port.
 * @param out Pointer to output the proxy port to.
 */
Result ACU_GetProxyPort(u32 *out);

/**
 * @brief Gets the connected network's proxy username.
 * @param username Pointer to output the proxy username to. (The size must be at least 0x20-bytes)
 */
Result ACU_GetProxyUserName(char *username);

/**
 * @brief Gets the connected network's proxy password.
 * @param password Pointer to output the proxy password to. (The size must be at least 0x20-bytes)
 */
Result ACU_GetProxyPassword(char *password);

/**
 * @brief Gets the last error to occur during a connection.
 * @param errorCode Pointer to output the error code to.
 */
Result ACU_GetLastErrorCode(u32* errorCode);

/**
 * @brief Gets the last detailed error to occur during a connection.
 * @param errorCode Pointer to output the error code to.
 */
Result ACU_GetLastDetailErrorCode(u32* errorCode);
