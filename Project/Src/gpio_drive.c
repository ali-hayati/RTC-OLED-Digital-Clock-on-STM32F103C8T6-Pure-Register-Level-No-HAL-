#include "gpio_drive.h"

/* Each GPIO pin in STM32F1 has 4 configuration bits:
* Bits [1:0] = MODE (output speed, or 00 for input)
* Bits [3:2] = CNF (pin type: Push-Pull/Open-Drain/...)
* Pins 0-7 -> CRL register
* Pins 8-15 -> CRH register
*/
/**
* @brief Write 4-bit configuration nibble (CNF[3:2] | MODE[1:0]) to CRL/CRH
* This shared function eliminates repetitive logic between different pin states.
*/
static void gpio_write_config_nibble(GPIO_TypeDef *port, uint8_t pin, uint32_t nibble)
{
    if (pin < 8U) {
        uint32_t shift = (uint32_t)pin * 4U;
        port->CRL &= ~(0xFUL << shift);
        port->CRL |= (nibble << shift);
    } else {
        uint32_t shift = ((uint32_t)pin - 8U) * 4U;
        port->CRH &= ~(0xFUL << shift);
        port->CRH |= (nibble << shift);
    }
}

static uint32_t gpio_speed_bits(uint8_t speed_mhz)
{
    switch (speed_mhz) {
        case 2:  return 0x2UL;
        case 50: return 0x3UL;
        case 10: /* fallthrough */
        default: return 0x1UL;
    }
}

void GPIO_ConfigOutputPP(GPIO_TypeDef *port, uint8_t pin, uint8_t speed_mhz)
{
	/* CNF = 00 (Output Push-Pull), so the nibble contains only MODE */
    uint32_t nibble = gpio_speed_bits(speed_mhz);
    gpio_write_config_nibble(port, pin, nibble);
}

void GPIO_ConfigAFOpenDrain(GPIO_TypeDef *port, uint8_t pin, uint8_t speed_mhz)
{
	/* CNF = 11 (Alternate-Function Open-Drain) in bits [3:2],
	* MODE in bits [1:0] */
    uint32_t nibble = (0x3UL << 2) | gpio_speed_bits(speed_mhz);
    gpio_write_config_nibble(port, pin, nibble);
}

void GPIO_ConfigInputPullUp(GPIO_TypeDef *port, uint8_t pin)
{
	/* CNF = 10 (Input with Pull-up/Pull-down)، MODE = 00 (ورودی) */
    uint32_t nibble = (0x2UL << 2);
    gpio_write_config_nibble(port, pin, nibble);

    /* In STM32F1, when CNF=10, the pull-up/pull-down direction is selected by the corresponding bit
    * in ODR: ODR=1 means Pull-Up, ODR=0 means Pull-Down */
    port->ODR |= (1UL << pin);
}

void GPIO_SetPin(GPIO_TypeDef *port, uint8_t pin)
{
    port->BSRR = (1UL << pin);
}

void GPIO_ResetPin(GPIO_TypeDef *port, uint8_t pin)
{
    port->BRR = (1UL << pin);
}

void GPIO_TogglePin(GPIO_TypeDef *port, uint8_t pin)
{
    if ((port->ODR & (1UL << pin)) != 0U) {
        GPIO_ResetPin(port, pin);
    } else {
        GPIO_SetPin(port, pin);
    }
}
