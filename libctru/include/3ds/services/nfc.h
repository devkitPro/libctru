/**
 * @file nfc.h
 * @brief NFC service.
 */
#pragma once

/**
 * @brief Initializes NFC.
 */
Result nfcInit(void);

/**
 * @brief Shuts down NFC.
 */
void nfcExit(void);

/**
 * @brief Gets the NFC service handle.
 * @return The NFC service handle.
 */
Handle nfcGetSessionHandle(void);

/**
 * @brief Initialize NFC module.
 * @param type Unknown, can be either value 0x1 or 0x2.
 */
Result NFC_Initialize(u8 type);

/**
 * @brief Shutdown NFC module.
 * @param type Unknown.
 */
Result NFC_Shutdown(u8 type);

/**
 * @brief O3DS starts communication with the O3DS NFC hardware. N3DS just checks state for this command.
 */
Result NFC_StartCommunication(void);

/**
 * @brief O3DS stops communication with the O3DS NFC hardware. N3DS just uses code used internally by NFC:StopTagScanning for this.
 */
Result NFC_StopCommunication(void);

/**
 * @brief Starts scanning for NFC tags.
 * @param unknown Unknown.
 */
Result NFC_StartTagScanning(u16 unknown);

/**
 * @brief Stops scanning for NFC tags.
 */
Result NFC_StopTagScanning(void);

/**
 * @brief Read Amiibo NFC data and load in memory.
 */
Result NFC_LoadAmiiboData(void);

/**
 * @brief If the tagstate is valid, it then sets the current tagstate to 3.
 */
Result NFC_ResetTagScanState(void);

/**
 * @brief Returns the current NFC tag state.
 * @param state Pointer to write NFC tag state.
 *
 * Tag state values:
 * - 0: NFC:Initialize was not used yet.
 * - 1: Not currently scanning for NFC tags. Set by NFC:StopTagScanning and NFC:Initialize, when successful.
 * - 2: Currently scanning for NFC tags. Set by NFC:StartTagScanning when successful.
 * - 3: NFC tag is in range. The state automatically changes to this when the state was previous value 3, without using any NFC service commands.
 * - 4: NFC tag is now out of range, where the NFC tag was previously in range. This occurs automatically without using any NFC service commands. Once this state is entered, it won't automatically change to anything else when the tag is moved in range again. Hence, if you want to keep doing tag scanning after this, you must stop+start scanning.
 * - 5: NFC tag data was successfully loaded. This is set by NFC:LoadAmiiboData when successful. 
 */
Result NFC_GetTagState(u8 *state);

