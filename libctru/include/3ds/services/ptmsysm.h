/**
 * @file ptmsysm.h
 * @brief PTMSYSM service.
 */
#pragma once

/// Initializes ptm:sysm.
Result ptmSysmInit(void);

/// Exits ptm:sysm.
void ptmSysmExit(void);


/**
 * @brief return 1 if it's a New 3DS otherwise, return 0 for Old 3DS.
 */
Result PTMSYSM_CheckNew3DS(void);

/**
 * @brief Configures the New 3DS' CPU clock speed and L2 cache.
 * @param value Bit0: enable higher clock, Bit1: enable L2 cache.
 */
Result PTMSYSM_ConfigureNew3DSCPU(u8 value);

/**
 * @brief Trigger a hardware system shutdown via the MCU
 * @param timeout: timeout passed to PMApp:TerminateNonEssential.
 */
Result PTMSYSM_ShutdownAsync(u64 timeout);

/**
 * @brief Trigger a hardware system reboot via the MCU.
 * @param timeout: timeout passed to PMApp:TerminateNonEssential.
 */
Result PTMSYSM_RebootAsync(u64 timeout);
