/**
 * @file cfgu.h
 * @brief CFGU (Configuration) Service
 */
#pragma once

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
	CFG_LANGUAGE_JP = 0,  ///< Japanese
	CFG_LANGUAGE_EN = 1,  ///< English
	CFG_LANGUAGE_FR = 2,  ///< French
	CFG_LANGUAGE_DE = 3,  ///< German
	CFG_LANGUAGE_IT = 4,  ///< Italian
	CFG_LANGUAGE_ES = 5,  ///< Spanish
	CFG_LANGUAGE_ZH = 6,  ///< Simplified Chinese
	CFG_LANGUAGE_KO = 7,  ///< Korean
	CFG_LANGUAGE_NL = 8,  ///< Dutch
	CFG_LANGUAGE_PT = 9,  ///< Portugese
	CFG_LANGUAGE_RU = 10, ///< Russian
	CFG_LANGUAGE_TW = 11, ///< Traditional Chinese
} CFG_Language;

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
 * @param model Pointer to output the model to. (0 = O3DS, 1 = O3DSXL, 2 = N3DS, 3 = 2DS, 4 = N3DSXL)
 */
Result CFGU_GetSystemModel(u8* model);

/**
 * @brief Gets whether the system is a 2DS.
 * @param value Pointer to output the result to. (0 = yes, 1 = no)
 */
Result CFGU_GetModelNintendo2DS(u8* value);

/**
 * @brief Gets a string representing a country code.
 * @param code Country code to use.
 * @param string Pointer to output the string to.
 */
Result CFGU_GetCountryCodeString(u16 code, u16* string);

/**
 * @brief Gets a country code ID from its string.
 * @param string String to use.
 * @param code Pointer to output the country code to.
 */
Result CFGU_GetCountryCodeID(u16 string, u16* code);

/**
 * @brief Gets a config info block with flags = 2.
 * @param size Size of the data to retrieve.
 * @param blkID ID of the block to retrieve.
 * @param outData Pointer to write the block data to.
 */
Result CFGU_GetConfigInfoBlk2(u32 size, u32 blkID, u8* outData);

/**
 * @brief Gets a config info block with flags = 4.
 * @param size Size of the data to retrieve.
 * @param blkID ID of the block to retrieve.
 * @param outData Pointer to write the block data to.
 */
Result CFG_GetConfigInfoBlk4(u32 size, u32 blkID, u8* outData);

/**
 * @brief Gets a config info block with flags = 8.
 * @param size Size of the data to retrieve.
 * @param blkID ID of the block to retrieve.
 * @param outData Pointer to write the block data to.
 */
Result CFG_GetConfigInfoBlk8(u32 size, u32 blkID, u8* outData);

/**
 * @brief Sets a config info block with flags = 4.
 * @param size Size of the data to retrieve.
 * @param blkID ID of the block to retrieve.
 * @param inData Pointer to block data to write.
 */
Result CFG_SetConfigInfoBlk4(u32 size, u32 blkID, u8* inData);

/**
 * @brief Sets a config info block with flags = 8.
 * @param size Size of the data to retrieve.
 * @param blkID ID of the block to retrieve.
 * @param inData Pointer to block data to write.
 */
Result CFG_SetConfigInfoBlk8(u32 size, u32 blkID, u8* inData);


/**
 * @brief Writes the CFG buffer in memory to the savegame in NAND.
 */
Result CFG_UpdateConfigNANDSavegame(void);

/**
 * @brief Gets the system's language.
 * @param language Pointer to write the language to. (see @ref CFG_Language)
 */
Result CFGU_GetSystemLanguage(u8* language);
