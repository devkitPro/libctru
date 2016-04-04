/**
 * @file ndm.h
 * @brief NDMU service. https://3dbrew.org/wiki/NDM_Services
 */
#pragma once

typedef enum {
	EXCLUSIVE_STATE_NONE = 0,
	EXCLUSIVE_STATE_INFRASTRUCTURE = 1,
	EXCLUSIVE_STATE_LOCAL_COMMUNICATIONS = 2,
	EXCLUSIVE_STATE_STREETPASS = 3,
	EXCLUSIVE_STATE_STREETPASS_DATA = 4,
} NDM_ExclusiveState;

/// Initializes ndmu.
Result ndmuInit(void);

/// Exits ndmu.
void ndmuExit(void);

Result ndmuEnterExclusiveState(NDM_ExclusiveState state);

Result ndmuLeaveExclusiveState(void);

