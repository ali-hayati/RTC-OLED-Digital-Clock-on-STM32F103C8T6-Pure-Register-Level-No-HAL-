/*
 * SSD1306_driver.h
 *
 *  Created on: Aug 29, 2026
 *      Author: ali Hayati
 */

#ifndef SSD1306_DRIVER_H_
#define SSD1306_DRIVER_H_

#include "stm32f103xb.h"
#define SSD1306_I2C_ADDR    0x3CU /* common address of most 128x64 modules (some are 0x3D) */

#define SSD1306_WIDTH       128U
#define SSD1306_HEIGHT      64U

#define SSD1306_COLOR_BLACK 0U
#define SSD1306_COLOR_WHITE 1U



/**
* @brief Full OLED initialization: Standard SSD1306 Init sequence + page clearing
* @return 0 on success, -1 if OLED does not respond on I2C bus (not connected/wrong address)
*/
int SSD1306_Init(void);

/**
* @brief Clear internal Frame Buffer (only in RAM; to apply to screen
* must be called after SSD1306_UpdateScreen)
*/
void SSD1306_Clear(void);

/**
* @brief Send complete Frame Buffer to OLED (1024 bytes in one I2C transaction)
* @return 0 on success, -1 on communication error
*/
int SSD1306_UpdateScreen(void);

/**
* @brief Turn on/off a pixel in the Frame Buffer (in RAM, not on the screen)
*/
void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color);

/**
* @brief Draw a character in a 5x7 font (with one column of white space after each character)
*/
void SSD1306_DrawChar(uint8_t x, uint8_t y, char c, uint8_t color);

/**
* @brief Draw a string with a 5x7 font (characters are arranged horizontally)
*/
void SSD1306_DrawString(uint8_t x, uint8_t y, const char *str, uint8_t color);


#endif /* SSD1306_DRIVER_H_ */
