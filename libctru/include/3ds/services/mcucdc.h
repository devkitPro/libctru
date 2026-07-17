/**
 * @file mcucdc.h
 * @brief MCU CODEC service.
 */
#pragma once

#include <3ds/types.h>
#include <3ds/services/mcu_common.h>

/// Initializes mcuCdc.
Result mcuCdcInit(void);

/// Exits mcuCdc.
void mcuCdcExit(void);

/**
 * @brief Gets the current mcuCdc session handle.
 * @return A pointer to the current mcuCdc session handle.
 */
Handle *mcuCdcGetSessionHandle(void);

/**
  * @brief Writes bit 4 to MCU register 26h.
  */
Result MCUCDC_SetReg26h();
