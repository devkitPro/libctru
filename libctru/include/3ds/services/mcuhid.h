/**
 * @file mcuhid.h
 * @brief MCU HID service.
 */
#pragma once

#include <3ds/types.h>
#include <3ds/services/mcu_common.h>

/// Initializes mcuHid.
Result mcuHidInit(void);

/// Exits mcuHid.
void mcuHidExit(void);

/**
 * @brief Gets the current mcuHid session handle.
 * @return A pointer to the current mcuHid session handle.
 */
Handle *mcuHidGetSessionHandle(void);

/**
  * @brief Sets the enabled sensors (pedometer, accelerometer) configuration.
  * @param config The configuration to use.
  */
Result MCUHID_SetSensorConfiguration(MCU_SensorConfig config);

/**
  * @brief Returns whether or not the accelerometer is enabled.
  * @param out_enabled Pointer to output whether or not the accelerometer is enabled.
  */
Result MCUHID_GetAccelerometerEnabled(bool *out_enabled);

/**
  * @brief Starts a raw accelerometer I2C read operation.
  * @param hw_regid The I2C register address to read from. Refer to the datasheet for the LIS331DLH IC for more information about the available registers.
  */
Result MCUHID_StartAccelerometerManualRead(u8 hw_regid);

/**
  * @brief Gets the result of an raw accelerometer I2C read operation.
  * @param out_data Pointer to output the read data byte to.
  */
Result MCUHID_GetAccelerometerManualReadResult(u8 *out_data);

/**
  * @brief Performs a raw accelerometer I2C write operation.
  * @param hw_regid The I2C register address to write to. Refer to the datasheet for the LIS331DLH IC for more information about the available registers.
  * @param data The data byte to write to the given address.
  */
Result MCUHID_PerformAccelerometerManualWrite(u8 hw_regid, u8 data);

/**
  * @brief Reads the current position data from the accelerometer.
  * @param out_data Pointer to output the accelerometer data to.
  */
Result MCUHID_ReadAccelerometerData(MCU_AccelerometerData *out_data);

/**
  * @brief Reads the current position of the 3D slider.
  * @param out_pos Pointer to output the position to.
  */
Result MCUHID_Read3dSliderPosition(u8 *out_pos);

/**
  * @brief Sets the scale of the accelerometer.
  * @param scale The new scale to use.
  */
Result MCUHID_SetAccelerometerScale(MCU_AccelerometerScale scale);

/**
  * @brief Gets the currently used scale of the accelerometer.
  * @param out_scale Pointer to output the scale to.
  */
Result MCUHID_GetAccelerometerScale(MCU_AccelerometerScale *out_scale);

/**
  * @brief Enables/disables the internal filter of the accelerometer. Refer to the datasheet for the LIS331DLH IC for more information.
  * @param enabled Whether or not the filter should be enabled.
  */
Result MCUHID_SetAccelerometerInternalFilterEnabled(bool enabled);

/**
  * @brief Returns whether or not the internal filter of the accelerometer is enabled. Refer to the datasheet for the LIS331DLH IC for more information.
  * @param out_enabled Pointer to output the status to.
  */
Result MCUHID_GetAccelerometerInternalFilterEnabled(bool *out_enabled);

/**
 * @brief Gets the event handle for MCUHID related events.
 * @param out_event Pointer to output the event handle to.
 */
Result MCUHID_GetEventHandle(Handle *out_handle);

/**
  * @brief Gets the HID events that have recently been received by the MCU.
  * @param out_events Pointer to output the received events to.
  */
Result MCUHID_GetReceivedEvents(u32 *out_events);

/**
 * @brief Gets the volume slider level.
 * @param level Pointer to write the slider level to.
 */
Result MCUHID_GetVolumeSliderLevel(u8 *level);

/**
  * @brief Enables/disables the IRQ that fires whenever the accelerometer publishes a new sample.
  * @param enable Whether or not to enable the IRQ.
  */
Result MCUHID_SetAccelerometerIrqEnabled(bool enable);