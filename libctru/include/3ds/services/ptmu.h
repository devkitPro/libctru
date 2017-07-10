/**
 * @file ptmu.h
 * @brief PTMU service.
 */
#pragma once

/// Initializes PTMU.
Result ptmuInit(void);

/// Exits PTMU.
void ptmuExit(void);

/**
 * @brief Gets the system's current shell state.
 * @param out Pointer to write the current shell state to. (0 = closed, 1 = open)
 */
Result PTMU_GetShellState(u8 *out);

/**
 * @brief Gets the system's current battery level.
 * @param out Pointer to write the current battery level to. (0-5)
 */
Result PTMU_GetBatteryLevel(u8 *out);

/**
 * @brief Gets the system's current battery charge state.
 * @param out Pointer to write the current battery charge state to. (0 = not charging, 1 = charging)
 */
Result PTMU_GetBatteryChargeState(u8 *out);

/**
 * @brief Gets the system's current pedometer state.
 * @param out Pointer to write the current pedometer state to. (0 = not counting, 1 = counting)
 */
Result PTMU_GetPedometerState(u8 *out);

/**
 * @brief Gets the pedometer's total step count.
 * @param steps Pointer to write the total step count to.
 */
Result PTMU_GetTotalStepCount(u32 *steps);

/**
 * @brief Gets whether the adapter is plugged in or not
 * @param out Pointer to write the adapter state to.
 */
Result PTMU_GetAdapterState(bool *out);
