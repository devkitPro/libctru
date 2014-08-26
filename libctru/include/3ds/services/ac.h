#pragma once

Result acInit();
Result acExit();

Result ACU_GetWifiStatus(Handle* servhandle, u32 *out);
Result ACU_WaitInternetConnection();
