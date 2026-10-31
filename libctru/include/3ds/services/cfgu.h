/**
 * @file cfgu.h
 * @brief CFGU (Configuration) Service
 */
#pragma once
#include <3ds/types.h>

/// Configuration region values.
typedef enum
{
	CFG_REGION_JPN = 0, ///< Japan
	CFG_REGION_USA = 1, ///< USA
	CFG_REGION_EUR = 2, ///< Europe
	CFG_REGION_AUS = 3, ///< Australia
	CFG_REGION_CHN = 4, ///< China
	CFG_REGION_KOR = 5, ///< Korea
	CFG_REGION_TWN = 6, ///< Taiwan
} CFG_Region;

/// Configuration language values.
typedef enum
{
	CFG_LANGUAGE_DEFAULT = -1, ///< Use system language in errorInit
	CFG_LANGUAGE_JP,           ///< Japanese
	CFG_LANGUAGE_EN,           ///< English
	CFG_LANGUAGE_FR,           ///< French
	CFG_LANGUAGE_DE,           ///< German
	CFG_LANGUAGE_IT,           ///< Italian
	CFG_LANGUAGE_ES,           ///< Spanish
	CFG_LANGUAGE_ZH,           ///< Simplified Chinese
	CFG_LANGUAGE_KO,           ///< Korean
	CFG_LANGUAGE_NL,           ///< Dutch
	CFG_LANGUAGE_PT,           ///< Portugese
	CFG_LANGUAGE_RU,           ///< Russian
	CFG_LANGUAGE_TW,           ///< Traditional Chinese
} CFG_Language;

// Configuration system model values.
typedef enum
{
	CFG_MODEL_3DS    = 0, ///< Old 3DS (CTR)
	CFG_MODEL_3DSXL  = 1, ///< Old 3DS XL (SPR)
	CFG_MODEL_N3DS   = 2, ///< New 3DS (KTR)
	CFG_MODEL_2DS    = 3, ///< Old 2DS (FTR)
	CFG_MODEL_N3DSXL = 4, ///< New 3DS XL (RED)
	CFG_MODEL_N2DSXL = 5, ///< New 2DS XL (JAN)
} CFG_SystemModel;

/// Configuration country values.
typedef enum
{
	CFG_COUNTRY_JP = 1,
	CFG_COUNTRY_AI = 8,
	CFG_COUNTRY_AG = 9,
	CFG_COUNTRY_AR = 10,
	CFG_COUNTRY_AW = 11,
	CFG_COUNTRY_BS = 12,
	CFG_COUNTRY_BB = 13,
	CFG_COUNTRY_BZ = 14,
	CFG_COUNTRY_BO = 15,
	CFG_COUNTRY_BR = 16,
	CFG_COUNTRY_VG = 17,
	CFG_COUNTRY_CA = 18,
	CFG_COUNTRY_KY = 19,
	CFG_COUNTRY_CL = 20,
	CFG_COUNTRY_CO = 21,
	CFG_COUNTRY_CR = 22,
	CFG_COUNTRY_DM = 23,
	CFG_COUNTRY_DO = 24,
	CFG_COUNTRY_EC = 25,
	CFG_COUNTRY_SV = 26,
	CFG_COUNTRY_GF = 27,
	CFG_COUNTRY_GD = 28,
	CFG_COUNTRY_GP = 29,
	CFG_COUNTRY_GT = 30,
	CFG_COUNTRY_GY = 31,
	CFG_COUNTRY_HT = 32,
	CFG_COUNTRY_HN = 33,
	CFG_COUNTRY_JM = 34,
	CFG_COUNTRY_MQ = 35,
	CFG_COUNTRY_MX = 36,
	CFG_COUNTRY_MS = 37,
	CFG_COUNTRY_AN = 38,
	CFG_COUNTRY_NI = 39,
	CFG_COUNTRY_PA = 40,
	CFG_COUNTRY_PY = 41,
	CFG_COUNTRY_PE = 42,
	CFG_COUNTRY_KN = 43,
	CFG_COUNTRY_LC = 44,
	CFG_COUNTRY_VC = 45,
	CFG_COUNTRY_SR = 46,
	CFG_COUNTRY_TT = 47,
	CFG_COUNTRY_TC = 48,
	CFG_COUNTRY_US = 49,
	CFG_COUNTRY_UY = 50,
	CFG_COUNTRY_VI = 51,
	CFG_COUNTRY_VE = 52,
	CFG_COUNTRY_AL = 64,
	CFG_COUNTRY_AU = 65,
	CFG_COUNTRY_AT = 66,
	CFG_COUNTRY_BE = 67,
	CFG_COUNTRY_BA = 68,
	CFG_COUNTRY_BW = 69,
	CFG_COUNTRY_BG = 70,
	CFG_COUNTRY_HR = 71,
	CFG_COUNTRY_CY = 72,
	CFG_COUNTRY_CZ = 73,
	CFG_COUNTRY_DK = 74,
	CFG_COUNTRY_EE = 75,
	CFG_COUNTRY_FI = 76,
	CFG_COUNTRY_FR = 77,
	CFG_COUNTRY_DE = 78,
	CFG_COUNTRY_GR = 79,
	CFG_COUNTRY_HU = 80,
	CFG_COUNTRY_IS = 81,
	CFG_COUNTRY_IE = 82,
	CFG_COUNTRY_IT = 83,
	CFG_COUNTRY_LV = 84,
	CFG_COUNTRY_LS = 85,
	CFG_COUNTRY_LI = 86,
	CFG_COUNTRY_LT = 87,
	CFG_COUNTRY_LU = 88,
	CFG_COUNTRY_MK = 89,
	CFG_COUNTRY_MT = 90,
	CFG_COUNTRY_ME = 91,
	CFG_COUNTRY_MZ = 92,
	CFG_COUNTRY_NA = 93,
	CFG_COUNTRY_NL = 94,
	CFG_COUNTRY_NZ = 95,
	CFG_COUNTRY_NO = 96,
	CFG_COUNTRY_PL = 97,
	CFG_COUNTRY_PT = 98,
	CFG_COUNTRY_RO = 99,
	CFG_COUNTRY_RU = 100,
	CFG_COUNTRY_RS = 101,
	CFG_COUNTRY_SK = 102,
	CFG_COUNTRY_SI = 103,
	CFG_COUNTRY_ZA = 104,
	CFG_COUNTRY_ES = 105,
	CFG_COUNTRY_SZ = 106,
	CFG_COUNTRY_SE = 107,
	CFG_COUNTRY_CH = 108,
	CFG_COUNTRY_TR = 109,
	CFG_COUNTRY_GB = 110,
	CFG_COUNTRY_ZM = 111,
	CFG_COUNTRY_ZW = 112,
	CFG_COUNTRY_AZ = 113,
	CFG_COUNTRY_MR = 114,
	CFG_COUNTRY_ML = 115,
	CFG_COUNTRY_NE = 116,
	CFG_COUNTRY_TD = 117,
	CFG_COUNTRY_SD = 118,
	CFG_COUNTRY_ER = 119,
	CFG_COUNTRY_DJ = 120,
	CFG_COUNTRY_SO = 121,
	CFG_COUNTRY_AD = 122,
	CFG_COUNTRY_GI = 123,
	CFG_COUNTRY_GG = 124,
	CFG_COUNTRY_IM = 125,
	CFG_COUNTRY_JE = 126,
	CFG_COUNTRY_MC = 127,
	CFG_COUNTRY_TW = 128,
	CFG_COUNTRY_KR = 136,
	CFG_COUNTRY_HK = 144,
	CFG_COUNTRY_MO = 145,
	CFG_COUNTRY_ID = 152,
	CFG_COUNTRY_SG = 153,
	CFG_COUNTRY_TH = 154,
	CFG_COUNTRY_PH = 155,
	CFG_COUNTRY_MY = 156,
	CFG_COUNTRY_CN = 160,
	CFG_COUNTRY_AE = 168,
	CFG_COUNTRY_IN = 169,
	CFG_COUNTRY_EG = 170,
	CFG_COUNTRY_OM = 171,
	CFG_COUNTRY_QA = 172,
	CFG_COUNTRY_KW = 173,
	CFG_COUNTRY_SA = 174,
	CFG_COUNTRY_SY = 175,
	CFG_COUNTRY_BH = 176,
	CFG_COUNTRY_JO = 177,
	CFG_COUNTRY_SM = 184,
	CFG_COUNTRY_VA = 185,
	CFG_COUNTRY_BM = 186,
} CFG_Country;

// Configuration country info structure.
typedef struct
{
	u8 unk[2];
	u8 state;
	u8 country;
} CFG_CountryInfo;

/// Initializes CFGU.
Result cfguInit(void);

/// Exits CFGU.
void cfguExit(void);

/**
 * @brief Gets the system's region from secure info.
 * @param region Pointer to output the region to. (see @ref CFG_Region)
 */
Result CFGU_SecureInfoGetRegion(u8* region);

/**
 * @brief Generates a console-unique hash.
 * @param appIDSalt Salt to use.
 * @param hash Pointer to output the hash to.
 */
Result CFGU_GenHashConsoleUnique(u32 appIDSalt, u64* hash);

/**
 * @brief Gets whether the system's region is Canada or USA.
 * @param value Pointer to output the result to. (0 = no, 1 = yes)
 */
Result CFGU_GetRegionCanadaUSA(u8* value);

/**
 * @brief Gets the system's model.
 * @param model Pointer to output the model to. (see @ref CFG_SystemModel)
 */
Result CFGU_GetSystemModel(u8* model);

/**
 * @brief Gets whether the system is a 2DS.
 * @param value Pointer to output the result to. (0 = yes, 1 = no)
 */
Result CFGU_GetModelNintendo2DS(u8* value);

/**
 * @brief Gets a string representing a country code.
 * @param code Country code to use. (see @ref CFG_Country)
 * @param string Pointer to output the string to. (must contain at least 2 bytes)
 */
Result CFGU_GetCountryCodeString(u16 code, char *string);

/**
 * @brief Gets a country code ID from its string.
 * @param string String to use.
 * @param code Pointer to output the country code to. (see @ref CFG_Country)
 */
Result CFGU_GetCountryCodeID(const char *string, u16* code);

/**
 * @brief Checks if NFC (code name: fangate) is supported.
 * @param isSupported pointer to the output the result to.
 */
Result CFGU_IsNFCSupported(bool* isSupported);

/**
 * @brief Gets a config info block with flags = 2.
 * @param size Size of the data to retrieve.
 * @param blkID ID of the block to retrieve.
 * @param outData Pointer to write the block data to.
 */
Result CFGU_GetConfigInfoBlk2(u32 size, u32 blkID, void* outData);

/**
 * @brief Gets a config info block with flags = 4.
 * @param size Size of the data to retrieve.
 * @param blkID ID of the block to retrieve.
 * @param outData Pointer to write the block data to.
 */
Result CFG_GetConfigInfoBlk4(u32 size, u32 blkID, void* outData);

/**
 * @brief Gets a config info block with flags = 8.
 * @param size Size of the data to retrieve.
 * @param blkID ID of the block to retrieve.
 * @param outData Pointer to write the block data to.
 */
Result CFG_GetConfigInfoBlk8(u32 size, u32 blkID, void* outData);

/**
 * @brief Sets a config info block with flags = 4.
 * @param size Size of the data to retrieve.
 * @param blkID ID of the block to retrieve.
 * @param inData Pointer to block data to write.
 */
Result CFG_SetConfigInfoBlk4(u32 size, u32 blkID, const void* inData);

/**
 * @brief Sets a config info block with flags = 8.
 * @param size Size of the data to retrieve.
 * @param blkID ID of the block to retrieve.
 * @param inData Pointer to block data to write.
 */
Result CFG_SetConfigInfoBlk8(u32 size, u32 blkID, const void* inData);


/**
 * @brief Writes the CFG buffer in memory to the savegame in NAND.
 */
Result CFG_UpdateConfigSavegame(void);

/**
 * @brief Gets the system's language.
 * @param language Pointer to write the language to. (see @ref CFG_Language)
 */
Result CFGU_GetSystemLanguage(u8* language);

/**
 * @brief Gets the system's country info.
 * @param language Pointer to write the country info to. (see @ref CFG_CountryInfo)
 */
Result CFGU_GetSystemCountryInfo(CFG_CountryInfo* country);

/**
 * @brief Deletes the NAND LocalFriendCodeSeed file, then recreates it using the LocalFriendCodeSeed data stored in memory.
 */
Result CFGI_RestoreLocalFriendCodeSeed(void);

/**
 * @brief Deletes the NAND SecureInfo file, then recreates it using the SecureInfo data stored in memory.
 */
Result CFGI_RestoreSecureInfo(void);

/**
 * @brief Deletes the "config" file stored in the NAND Config_Savegame.
 */
Result CFGI_DeleteConfigSavefile(void);

/**
 * @brief Formats Config_Savegame.
 */
Result CFGI_FormatConfig(void);

/**
 * @brief Clears parental controls
 */
Result CFGI_ClearParentalControls(void);

/**
 * @brief Verifies the RSA signature for the LocalFriendCodeSeed data already stored in memory.
 */
Result CFGI_VerifySigLocalFriendCodeSeed(void);

/**
 * @brief Verifies the RSA signature for the SecureInfo data already stored in memory.
 */
Result CFGI_VerifySigSecureInfo(void);

/**
 * @brief Gets the system's serial number.
 * @param serial Pointer to output the serial to. (This is normally 0xF)
 */
Result CFGI_SecureInfoGetSerialNumber(u8 *serial);

/**
 * @brief Gets the 0x110-byte buffer containing the data for the LocalFriendCodeSeed.
 * @param data Pointer to output the buffer. (The size must be at least 0x110-bytes)
 */
Result CFGI_GetLocalFriendCodeSeedData(u8 *data);

/**
 * @brief Gets the 64-bit local friend code seed.
 * @param seed Pointer to write the friend code seed to.
 */
Result CFGI_GetLocalFriendCodeSeed(u64* seed);

/**
 * @brief Gets the 0x11-byte data following the SecureInfo signature.
 * @param data Pointer to output the buffer. (The size must be at least 0x11-bytes)
 */
Result CFGI_GetSecureInfoData(u8 *data);

/**
 * @brief Gets the 0x100-byte RSA-2048 SecureInfo signature.
 * @param data Pointer to output the buffer. (The size must be at least 0x100-bytes)
 */
Result CFGI_GetSecureInfoSignature(u8 *data);
