/**
 * @file sdmc.h
 * @brief SDMC driver.
 */
#pragma once

#include <3ds/types.h>

/// Initializes the SDMC driver.
Result sdmcInit(void);

/// Enable/disable copy in sdmc_write
void sdmcWriteSafe(bool enable);

/// Exits the SDMC driver.
Result sdmcExit(void);
