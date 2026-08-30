#ifndef SYSTICK_DRIVER_H
#define SYSTICK_DRIVER_H

#include "stm32f103xb.h"

/**
* @brief Start SysTick in interrupt mode with a tick of exactly 1ms
* (based on AHB core clock = 72MHz). This function should be called only once, after
* RCC_SystemClockConfig().
*
* Unlike the previous version of the project (Busy-Wait on COUNTFLAG), this version maintains a global millisecond counter (Uptime) that can be used for both delay and
* button press duration (Debounce/Long-Press) -
* without locking SysTick exclusively.
*/
void SysTick_InitTick(void);

/**
* @brief Read the milliseconds counter since boot (Uptime)
*/
uint32_t SysTick_GetTick(void);

/**
* @brief Non-blocking delay based on Uptime counter (not direct Busy-Wait
* on SysTick register; other interrupts like EXTI button also
* work correctly during the delay)
* @param ms Delay duration in milliseconds
*/
void SysTick_DelayMs(uint32_t ms);

#endif /* SYSTICK_DRIVER_H */
