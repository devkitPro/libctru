/**
 * @file sdmc.h
 * @brief SDMC driver.
 */
#pragma once

#include <3ds/types.h>

/**
 * @brief Initializes the SDMC driver.
 */
Result sdmcInit(void);

/**
 * @brief Exits the SDMC driver.
 */
Result sdmcExit(void);
