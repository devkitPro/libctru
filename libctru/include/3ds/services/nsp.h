/**
 * @file nsp.h
 * @brief NS (Nintendo Shell) Power Service.
 */
#pragma once

/// Initializes NSP.
Result nspInit(void);

/// Exits NSP.
void nspExit(void);

/**
 * @brief Reboots system
 * @param launchtitle Whether to launch current title after reboot. true = launch, false = do not launch
 */
Result NSP_RebootSystem(bool launchtitle);

/**
 * @brief Shutdowns system with a internal timeout of 14s
 */
Result NSP_ShutdownAsync(void);