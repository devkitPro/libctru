/**
 * @file ptmgets.h
 * @brief PTMGETS service.
 */
#pragma once

#include <3ds/types.h>

/// Initializes PTMGETS.
Result ptmGetsInit(void);

/// Exits PTMGETS.
void ptmGetsExit(void);

/**
 * @brief Gets a pointer to the current ptm:gets session handle.
 * @return A pointer to the current ptm:gets session handle.
 */
Handle *ptmGetsGetSessionHandle(void);

/**
 * @brief Gets the system time.
 * @param[out] outMsY2k The pointer to write the number of milliseconds since 01/01/2000 to.
 */
Result PTMGETS_GetSystemTime(s64 *outMsY2k);
