/*
 * rcc_driver.h
 *
 *  Created on: Aug 29, 2026
 *      Author: ali Hayati
 */

#ifndef RCC_DRIVER_H
#define RCC_DRIVER_H
#include "stm32f103xb.h"

/* Target : HSE(8MHz) -> PLL x9 -> SYSCLK = 72MHz
 *   AHB  = 72MHz
 *   APB1 = 36MHz  (related to Datasheet : 36 MHZ is Max)
 *   APB2 = 72MHz
 */
#define HSE_FREQ_HZ       8000000UL
#define SYSCLK_FREQ_HZ    72000000UL
/**
* @brief Complete system clock tree initialization (HSE + PLL + Flash Latency + Bus Prescalers)
* @return 0 on success
* -1 on timeout when HSE is ready or PLL is locked
*
* Note: In case of error the system will still remain on HSI (default internal clock 8MHz)
* and will not crash; final decision (e.g. flashing warning) is up to main.
*/
int RCC_SystemClockConfig(void);
/**
* @brief Enable AFIO clock and GPIO ports used in the project (A, B, C)
*/
void RCC_EnableGPIOClocks(void);
/**
* @brief PWR and BKP clock activation - mandatory prerequisite for RTC domain access (phase 2)
*/
void RCC_EnablePWR_BKP_Clocks(void);

/**
* @brief Enable I2C1 clock - mandatory prerequisite for communicating with OLED (Phase 3)
*/
void RCC_EnableI2C1Clock(void);

#endif/* RCC_DRIVER_H */
