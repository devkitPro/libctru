/**
 * @file ptm.h
 * @brief PTM service.
 */
#pragma once

/// Initializes PTM.
Result ptmInit(void);

/// Exits PTM.
Result ptmExit(void);

/**
 * @brief Gets the system's current shell state.
 * @param servhandle Optional pointer to the handle to use.
 * @param out Pointer to write the current shell state to. (0 = closed, 1 = open)
 */
Result PTMU_GetShellState(Handle* servhandle, u8 *out);

/**
 * @brief Gets the system's current battery level.
 * @param servhandle Optional pointer to the handle to use.
 * @param out Pointer to write the current battery level to. (0-5)
 */
Result PTMU_GetBatteryLevel(Handle* servhandle, u8 *out);

/**
 * @brief Gets the system's current battery charge state.
 * @param servhandle Optional pointer to the handle to use.
 * @param out Pointer to write the current battery charge state to. (0 = not charging, 1 = charging)
 */
Result PTMU_GetBatteryChargeState(Handle* servhandle, u8 *out);

/**
 * @brief Gets the system's current pedometer state.
 * @param servhandle Optional pointer to the handle to use.
 * @param out Pointer to write the current pedometer state to. (0 = not counting, 1 = counting)
 */
Result PTMU_GetPedometerState(Handle* servhandle, u8 *out);

/**
 * @brief Gets the pedometer's total step count.
 * @param servhandle Optional pointer to the handle to use.
 * @param steps Pointer to write the total step count to.
 */
Result PTMU_GetTotalStepCount(Handle* servhandle, u32 *steps);
