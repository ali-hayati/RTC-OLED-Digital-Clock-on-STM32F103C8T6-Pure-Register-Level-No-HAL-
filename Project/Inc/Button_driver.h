/*
 * Button_driver.h
 *
 *  Created on: Aug 31, 2026
 *      Author: ali Hayati
 */

#ifndef BUTTON_DRIVER_H_
#define BUTTON_DRIVER_H_

#include "stm32f103xb.h"

/* Button on PA0, Active-Low (one end to GND, the other end to PA0; internal pull-up
* is active). This is EXTI Line0 pin, which has its own independent IRQ
* (unlike EXTI5..9 and EXTI10..15 which share it) - an easy choice for
* the first button implementation. */
#define BUTTON_PORT   GPIOA
#define BUTTON_PIN    0U

/* Minimum time interval between two valid basic state changes, for mechanical vibration filter
* Button contact (Debounce). Typical values ​​20-50ms for Tact Switch buttons. */
#define BUTTON_DEBOUNCE_MS     30UL

/* Threshold for detecting high pressure against short pressure */
#define BUTTON_LONG_PRESS_MS   800UL

typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_SHORT_PRESS,
    BUTTON_EVENT_LONG_PRESS
} button_event_t;

/**
* @brief Complete button path configuration: GPIO Input Pull-Up + AFIO EXTI Routing
* + Enable interrupt on both edges (RTSR+FTSR) + Enable NVIC
*
* Note: This function must be called after RCC_EnableGPIOClocks() and after
* SysTick_InitTick(), because the internal ISR needs
* SysTick_GetTick() for Debounce.
*/
void Button_Init(void);

/**
* @brief Read and clear pending event (must be polled in main loop)
* The short/long detection logic is done entirely inside the ISR; this function just
* reads a simple, lock-free flag (Single-writer in ISR / Single-reader
* in main) - no need for a Critical Section
* since each event is only written/read by one party.
*/
button_event_t Button_GetEvent(void);

#endif /* BUTTON_DRIVER_H */


