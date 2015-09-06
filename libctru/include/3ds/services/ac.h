#pragma once

Result acInit(void);
Result acExit(void);

Result ACU_GetWifiStatus(Handle* servhandle, u32 *out);
Result ACU_WaitInternetConnection(void);
