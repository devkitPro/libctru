/**
 * @file irrst.h
 * @brief IRRST service.
 */
#pragma once

//See also: http://3dbrew.org/wiki/IR_Services http://3dbrew.org/wiki/IRRST_Shared_Memory

#include "3ds/services/hid.h" // for circlePosition definition

/// IRRST's shared memory handle.
extern Handle irrstMemHandle;

/// IRRST's shared memory.
extern vu32* irrstSharedMem;

/// IRRST's state update event
extern Handle irrstEvent;

/// Initializes IRRST.
Result irrstInit(void);

/// Exits IRRST.
void irrstExit(void);

/// Scans IRRST for input.
void irrstScanInput(void);

/**
 * @brief Gets IRRST's held keys.
 * @return IRRST's held keys.
 */
u32 irrstKeysHeld(void);

/**
 * @brief Reads the current c-stick position.
 * @param pos Pointer to output the current c-stick position to.
 */
void irrstCstickRead(circlePosition* pos);

/**
 * @brief Waits for the IRRST input event to trigger.
 * @param nextEvent Whether to discard the current event and wait until the next event.
 */
void irrstWaitForEvent(bool nextEvent);

/// Macro for irrstCstickRead.
#define hidCstickRead irrstCstickRead

/**
 * @brief Gets the shared memory and event handles for IRRST.
 * @param outMemHandle Pointer to write the shared memory handle to.
 * @param outEventHandle Pointer to write the event handle to.
 */
Result IRRST_GetHandles(Handle* outMemHandle, Handle* outEventHandle);

/**
 * @brief Initializes IRRST.
 * @param unk1 Unknown.
 * @param unk2 Unknown.
 */
Result IRRST_Initialize(u32 unk1, u8 unk2);

/// Shuts down IRRST.
Result IRRST_Shutdown(void);
