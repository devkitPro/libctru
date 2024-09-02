/**
 * @file qtmc.h
 * @brief QTM Hardware Check service.
 *
 * Allows direct control over the parallax barrier's pattern and polarity phase through the
 * TI TCA6416A I2C->Parallel expander, as well as control over IR LED state even when camera
 * is used by user.
 *
 * TI TCA6416A I2C->Parallel expander is located on bus I2C1 (PA 0x10161000) device ID 0x40.
 *
 * The top screen parallax barrier was covered by patent US20030234980A1.
 */
#pragma once

#include <3ds/types.h>

/**
 * @brief Initializes `qtm:c`.
 *        Only 3 sessions (2 until 9.3.0 sysupdate) for ALL services COMBINED, including the main
 *        services, can be open at a time.
 */
Result qtmcInit(void);

/// Exits `qtm:c`.
void qtmcExit(void);

/// Returns a pointer to the current `qtm:c` session handle.
Handle *qtmcGetSessionHandle(void);

/**
 * @brief  Starts the QTM Hardware Check API. This must be called before using any other `qtm:c` command,
 *         and causes barrier pattern to be overriden by what was last set in \ref QTMC_SetBarrierPattern,
 *         **even in 2D mode**. Also allows IR LED state to be overridden even if user uses the inner camera.
 * @return `0xD82183F9` if already started, otherwise 0 (success).
 */
Result QTMC_StartHardwareCheck(void);

/**
 * @brief  Stops the QTM Hardware Check API. Restore normal barrier and IR LED management behavior.
 * @return `0xD82183F8` if API not started, otherwise 0 (success).
 */
Result QTMC_StopHardwareCheck(void);

/**
 * @brief  Sets the parallax barrier's mask pattern and polarity phase (12+1 bits).
 *
 *         Bit11 to 0 correspond to a repeating barrier mask pattern, 0 meaning the corresponding mask unit is
 *         transparent and 1 that it is opaque. The direction is: left->right corresponds to MSB->LSB.
 *
 *         Bit12 is the polarity bit.
 *
 *         QTM's expander management thread repeatedly writes (on every loop iteration) the current mask pattern
 *         plus polarity bit, whether it is normally set or overridden by `qtm:c`, then on the following set,
 *         negates both (it writes pattern ^ 0x1FFF). This is done at all times, even it 2D mode.
 *
 *         The register being written to are regId 0x02 and 0x03 (output ports).
 *         TI TCA6416A I2C->Parallel expander is located on bus I2C1 (PA 0x10161000) device ID 0x40.
 *
 *         This function has no effect on N2DSXL.
 *
 * @param pattern Barrier mask pattern (bit12: polarity, bit11-0: 12-bit mask pattern)
 * @return `0xD82183F8` if API not started, otherwise 0 (success).
 * @see    Patent US20030234980A1 for a description of parallax barriers.
 * @example The mask pattern used for super-stable 3D are as follows (position 0 to 11):
 *
 *              000011111100
 *              000001111110
 *              000000111111
 *              100000011111
 *              110000001111
 *              111000000111
 *              111100000011
 *              111110000001
 *              111111000000
 *              011111100000
 *              001111110000
 *              000111111000
 *
 * When SS3D is disabled (ie. it tries to match O3DS behavior), then pattern becomes:
 *              111100000111
 * Notice that the slit width is reduced from 6 to 5 units there.
 *
 * For 2D it is all-zero:
 *              000000000000
 *
 * 2D pattern is automatically set on QTM process init and exit.
 */
Result QTMC_SetBarrierPattern(u32 pattern);

/**
 * @brief  Waits for the expander management thread to (re)initalize the TI TCA6416A I2C->Parallel expander,
 *         then checks if that expander is behaving as expected (responds with the port direction config
 *         it has been configured with): it checks whether all ports have been configured as outputs.
 *
 *         On N2DSXL, this function waits forever and never returns.
 *
 *         In detail, the hardware init procedure for the expander is as follows (as done by the expander mgmt. thread):
 *             - configure enable expander pin on SoC: set GPIO3.bit11 to OUTPUT, then set to 1
 *             - on the expander (I2C1 deviceId 0x40), set all ports to OUTPUT (regId 0x06, 0x07)
 *             - on the expander, write 0 (all-transparent mask/2D) to the data registers (regId 0x02, 0x03)
 *
 * @param[out] outWorking Where to write the working status to. If true, expander is present working.
 *                        If false, the expander is present but is misbehaving. If the function does not
 *                        return, then expander is missing (e.g. on N2DSXL).
 * @return `0xD82183F8` if API not started, otherwise 0 (success).
 */
Result QTMC_WaitAndCheckExpanderWorking(bool *outWorking);

/**
 * @brief  Temporarily overrides IR LED state. Requires "manual control" from `qtm:u` to be disabled, and has
 *         lower priority than it. Same implementation as \ref QTMS_SetIrLedStatusOverride.
 *
 * @param on Whether to turn the IR LED on or off.
 * @return `0xD82183F8` if API not started, `0xC8A18005` if manual control was enabled or if the operation failed,
 *         or `0xC8A18009` if QTM is unavailable (in particular, QTM is always unavailable on N2DSXL). Otherwise, 0 (success).
 */
Result QTMC_SetIrLedStatusOverride(bool on);
