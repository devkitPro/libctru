#pragma once

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
