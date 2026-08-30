#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include "stm32f103xb.h"

/* In this phase only the Push-Pull output is implemented (for LED testing).
* In phase 3, Open-Drain mode (for I2C) and input with Pull-up/EXTI are added. */

/**
* @brief Configure a pin as a Push-Pull output
* @param port Port pointer (e.g. GPIOC)
* @param pin Pin number (0..15)
* @param speed_mhz Allowed output speed: 2, 10 or 50 MHz
*/
void GPIO_ConfigOutputPP(GPIO_TypeDef *port, uint8_t pin, uint8_t speed_mhz);

/**
* @brief Configure a pin as Alternate-Function Open-Drain
* (Mandatory mode for SCL/SDA pins in I2C: the bus line is held high by an external/internal pull-up
* and the pin can only be connected to GND
* - it should never be Push-Pull, otherwise there is a possibility of a short circuit with collision of two masters or
* specific bus modes)
* @param port Port pointer (e.g. GPIOB for I2C1)
* @param pin Pin number (0..15)
* @param speed_mhz Allowed output speed: 2, 10 or 50 MHz
*/
void GPIO_ConfigAFOpenDrain(GPIO_TypeDef *port, uint8_t pin, uint8_t speed_mhz);

/**
* @brief Configure a pin as an input with internal Pull-Up
* (suitable for a button with one end connected to GND and the other end to this pin
* ; in the released state, the pin is held HIGH by the internal pull-up
* and goes LOW when the button is pressed - i.e. Active-Low)
* @param port Port pointer
* @param pin Pin number (0..15)
*/
void GPIO_ConfigInputPullUp(GPIO_TypeDef *port, uint8_t pin);

void GPIO_SetPin(GPIO_TypeDef *port, uint8_t pin);
void GPIO_ResetPin(GPIO_TypeDef *port, uint8_t pin);
void GPIO_TogglePin(GPIO_TypeDef *port, uint8_t pin);

#endif /* GPIO_DRIVER_H */
