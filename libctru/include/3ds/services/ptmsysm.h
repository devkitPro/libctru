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
 * @brief Configures the New 3DS' CPU clock speed and L2 cache.
 * @param value Bit0: enable higher clock, Bit1: enable L2 cache.
 */
Result PTMSYSM_ConfigureNew3DSCPU(u8 value);

