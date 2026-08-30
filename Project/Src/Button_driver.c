/*
 * Button_driver.c
 *
 *  Created on: Aug 31, 2026
 *      Author: ali Hayati
 */

#include "button_driver.h"
#include "gpio_drive.h"
#include "systick_deriver.h"

/* --- Internal state: written only by ISR, read by main --- */
static volatile uint32_t      s_last_edge_tick   = 0UL;
static volatile uint32_t      s_press_start_tick = 0UL;
static volatile uint8_t       s_is_pressed       = 0U;
static volatile button_event_t s_pending_event   = BUTTON_EVENT_NONE;

void Button_Init(void)
{
    GPIO_ConfigInputPullUp(BUTTON_PORT, BUTTON_PIN);

    /* AFIO should tell which port EXTI Line0 is connected to. The value 0000 means
    * PA0 (default); this is explicitly set to zero so that if the button is ever moved to a different
    * port, this line will be updated accordingly. */
    AFIO->EXTICR[0] &= ~(0xFUL << 0);

    EXTI->RTSR |= EXTI_RTSR_TR0;  /* Rising edge -> button release detection */
    EXTI->FTSR |= EXTI_FTSR_TR0; /* Landing edge -> button press detection */
    EXTI->PR    = EXTI_PR_PR0;   /* Clear any old/fake flags */
    EXTI->IMR  |= EXTI_IMR_MR0; /* Unmask this line's break */

    NVIC_EnableIRQ(EXTI0_IRQn);
}

button_event_t Button_GetEvent(void)
{
    button_event_t event = s_pending_event;
    s_pending_event = BUTTON_EVENT_NONE;
    return event;
}

/**
* @brief ISR Button - name must exactly match the EXTI0 entry in the Vector Table
* . In the STM32CubeIDE project, this handler is already defined as Weak in
* the startup file; this definition overrides it here.
*
* Implemented logic (fully non-blocking, no delay loop):
* 1) Debounce filter based on real time interval from SysTick (not counting
* instructions) - robust to clock frequency changes or compiler optimizations
* 2) Falling edge (base goes LOW) = start push; rising edge (base goes HIGH) =
* end push -> calculate duration and classify into short/long
*/
void EXTI0_IRQHandler(void)
{
    if ((EXTI->PR & EXTI_PR_PR0) == 0U) {
        return; /* This IRQ is not woken up by another line (extra precaution) */
    }
    EXTI->PR = EXTI_PR_PR0; /* Clear the flag (writing 1 will set it to zero) */

    uint32_t now = SysTick_GetTick();

    if ((now - s_last_edge_tick) < BUTTON_DEBOUNCE_MS) {
        return; /* Contact vibration noise - ignored */
    }
    s_last_edge_tick = now;

    uint8_t pin_is_low = ((BUTTON_PORT->IDR & (1UL << BUTTON_PIN)) == 0U) ? 1U : 0U;

    if (pin_is_low != 0U) {
    	/* Button just pressed */
        s_press_start_tick = now;
        s_is_pressed = 1U;
    } else if (s_is_pressed != 0U) {
    	/* Button released -> duration of press determines whether it was short or long */
        uint32_t duration = now - s_press_start_tick;
        s_pending_event = (duration >= BUTTON_LONG_PRESS_MS)
                               ? BUTTON_EVENT_LONG_PRESS
                               : BUTTON_EVENT_SHORT_PRESS;
        s_is_pressed = 0U;
    }
}
