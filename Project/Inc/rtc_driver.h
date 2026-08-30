/*
 * rtc_driver.h
 *
 *  Created on: Aug 29, 2026
 *      Author: ali Hayati
 */

#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

#include "stm32f103xb.h"
/* ==========================================================================
 * Internal RTC Driver for STM32F103 (LSE = 32.768 kHz, 1-second accuracy)
 * ==========================================================================
 * Important Note: Unlike newer families (such as STM32F4), the STM32F1
 * RTC does not have an internal calendar (day/month/year). It is only a
 * 32-bit counter that increments by one every second. Converting this raw
 * counter value into date/time is handled separately by the
 * calendar_convert module.
 * ==========================================================================
 */

/**
 * @brief Fully initializes the RTC: unlocks the Backup domain, enables LSE,
 *        selects LSE as the RTC clock source, enables the RTC, and configures
 *        the prescaler for an accurate 1-second time base.
 *
 * Prerequisite: RCC_EnablePWR_BKP_Clocks() must be called before this
 * function (the PWR and BKP clocks are mandatory for accessing the
 * Backup domain).
 *
 * @return 0 on success, -1 on timeout (e.g., if the LSE crystal is not
 *         installed or is not simulated).
 */
int RTC_Init(void);
/**
* @brief Write a desired value to the RTC counter (for initial time/date setting)
* @param unix_seconds Value in seconds from a desired epoch (project proposal: Unix epoch)
*/
void RTC_SetCounter(uint32_t unix_seconds);
/**
* @brief Read the current value of the RTC counter (race-safe read between CNTH/CNTL)
*/
uint32_t RTC_GetCounter(void);
/**
* @brief Check "every second" flag (SECF) - for use in main loop (Polling)
*/
uint8_t RTC_SecondFlagIsSet(void);
/**
* @brief Clear SECF flag (must be explicitly set to zero immediately after processing)
*/
void RTC_ClearSecondFlag(void);

#endif /* RTC_DRIVER_H */
