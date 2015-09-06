#pragma once

Result ptmInit(void);
Result ptmExit(void);

Result PTMU_GetShellState(Handle* servhandle, u8 *out);
Result PTMU_GetBatteryLevel(Handle* servhandle, u8 *out);
Result PTMU_GetBatteryChargeState(Handle* servhandle, u8 *out);
Result PTMU_GetPedometerState(Handle* servhandle, u8 *out);
Result PTMU_GetTotalStepCount(Handle* servhandle, u32 *steps);
