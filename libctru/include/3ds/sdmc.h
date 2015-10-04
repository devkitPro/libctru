/**
 * @file sdmc.h
 * @brief SDMC driver.
 */
#pragma once

#include <3ds/types.h>

/// Initializes the SDMC driver.
Result sdmcInit(void);

/// Exits the SDMC driver.
Result sdmcExit(void);
