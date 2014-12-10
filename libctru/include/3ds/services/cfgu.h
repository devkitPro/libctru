#pragma once

Result initCfgu(void);
Result exitCfgu(void);

Result CFGU_GetRegionCanadaUSA(u8* value);
Result CFGU_GetSystemModel(u8* model);
Result CFGU_GetModelNintendo2DS(u8* value);
Result CFGU_GetCountryCodeString(u16 code, u16* string);
Result CFGU_GetCountryCodeID(u16 string, u16* code);
