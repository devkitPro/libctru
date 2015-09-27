#pragma once

typedef enum
{
	CFG_REGION_JPN = 0,
	CFG_REGION_USA = 1,
	CFG_REGION_EUR = 2,
	CFG_REGION_AUS = 3,
	CFG_REGION_CHN = 4,
	CFG_REGION_KOR = 5,
	CFG_REGION_TWN = 6,
} CFG_Region;

typedef enum
{
	CFG_LANGUAGE_JP = 0,
	CFG_LANGUAGE_EN = 1,
	CFG_LANGUAGE_FR = 2,
	CFG_LANGUAGE_DE = 3,
	CFG_LANGUAGE_IT = 4,
	CFG_LANGUAGE_ES = 5,
	CFG_LANGUAGE_ZH = 6,
	CFG_LANGUAGE_KO = 7,
	CFG_LANGUAGE_NL = 8,
	CFG_LANGUAGE_PT = 9,
	CFG_LANGUAGE_RU = 10,
	CFG_LANGUAGE_TW = 11,
} CFG_Langage;

Result initCfgu(void);
Result exitCfgu(void);

Result CFGU_SecureInfoGetRegion(u8* region);
Result CFGU_GenHashConsoleUnique(u32 appIDSalt, u64* hash);
Result CFGU_GetRegionCanadaUSA(u8* value);
Result CFGU_GetSystemModel(u8* model);
Result CFGU_GetModelNintendo2DS(u8* value);
Result CFGU_GetCountryCodeString(u16 code, u16* string);
Result CFGU_GetCountryCodeID(u16 string, u16* code);
Result CFGU_GetConfigInfoBlk2(u32 size, u32 blkID, u8* outData);
Result CFGU_GetSystemLanguage(u8* language);
