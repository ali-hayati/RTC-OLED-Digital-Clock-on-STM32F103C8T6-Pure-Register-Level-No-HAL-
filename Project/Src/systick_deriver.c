#include "systick_deriver.h"
#include "rcc_driver.h"


static volatile uint32_t s_tick_ms = 0UL;
/* Global counter in milliseconds since boot. volatile because it is used both in ISR (write)
* and main (read) code. */
void SysTick_InitTick(void)
{
	/* Reload for exactly 1 millisecond with 72MHz core clock */
    SysTick->LOAD = (SYSCLK_FREQ_HZ / 1000UL) - 1UL;
    SysTick->VAL  = 0UL;

    /* Unlike the previous version, this time TICKINT is also enabled so that SysTick will interrupt every
    * 1ms and increment s_tick_ms - as a result, the CPU is free during
    * counting and can also handle other interrupts (e.g. button EXTI); unlike the previous Busy-Wait which exclusively
    * occupied the entire delay time. */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_TICKINT_Msk   |
                     SysTick_CTRL_ENABLE_Msk;
}

uint32_t SysTick_GetTick(void)
{
    return s_tick_ms;
}

void SysTick_DelayMs(uint32_t ms)
{
    uint32_t start = SysTick_GetTick();
    /* uint32 subtraction naturally handles 32-bit overflow (wrap-around every ~49 days)
    * properly, as it is correct in modulo arithmetic 2^32 */
    while ((SysTick_GetTick() - start) < ms) {
    	/* Meanwhile, other interrupts (EXTI, and later RTC_IRQ) ​​are executed normally */
    }
}

/**
* @brief Official SysTick ISR - its name must be exactly the same as the corresponding entry in the Vector
* Table (in STM32CubeIDE projects this handler is already defined as Weak in
* the startup file and is overridden here; in
* the Makefile of this repo itself, the exact same name is defined as Weak in startup_stm32f103xb.s
* ).
*/
void SysTick_Handler(void)
{
    s_tick_ms++;
}
