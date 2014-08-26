#pragma once

Result ptmInit();
Result ptmExit();

Result PTMU_GetBatteryLevel(Handle* servhandle, u8 *out);
Result PTMU_GetBatteryChargeState(Handle* servhandle, u8 *out);
