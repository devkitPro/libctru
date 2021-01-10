/**
 * @file ptmsets.h
 * @brief PTMSETS service.
 */
#pragma once

#include <3ds/types.h>

/// Initializes PTMSETS.
Result ptmSetsInit(void);

/// Exits PTMSETS.
void ptmSetsExit(void);

/**
 * @brief Gets a pointer to the current ptm:sets session handle.
 * @return A pointer to the current ptm:sets session handle.
 */
Handle *ptmSetsGetSessionHandle(void);

/**
 * @brief Sets the system time.
 * @param msY2k The number of milliseconds since 01/01/2000.
 */
Result PTMSETS_SetSystemTime(s64 msY2k);
