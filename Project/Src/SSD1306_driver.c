/*
 * SSD1306_deriver.c
 *
 *  Created on: Aug 29, 2026
 *      Author: ali Hayati
 */
#define DEBUG  0
#include "SSD1306_driver.h"
#include "i2c_driver.h"
#include "font.h"
#if DEBUG
	#include "Uart.h"
	#include "stdio.h"
#endif
#define SSD1306_CTRL_CMD    0x00U /* Co=0, D/C=0 -> next byte is Command */
#define SSD1306_CTRL_DATA   0x40U /* Co=0, D/C=1 -> next bytes are Data */

/* Frame Buffer: 128x64 monochrome pixels = 1024 bytes (8 128-byte pages) */
static uint8_t s_framebuffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8U];

static int ssd1306_write_command(uint8_t cmd)
{
    uint8_t buf[2] = {SSD1306_CTRL_CMD, cmd};
    return I2C_WriteBuffer(SSD1306_I2C_ADDR, buf, 2U);
}

int SSD1306_Init(void)
{
	 static const uint8_t init_cmds[] = {
	        0xAEU,             /* Display OFF                              */
	        0xD5U, 0x80U,      /* Clock Divide Ratio / Oscillator Frequency */
	        0xA8U, 0x3FU,      /* Multiplex Ratio = 64                     */
	        0xD3U, 0x00U,      /* Display Offset = 0                       */
	        0x40U,             /* Display Start Line = 0                   */
	        0x8DU, 0x14U,      /* Charge Pump Enable (internal)                */
	        0x20U, 0x00U,      /* Memory Addressing Mode = Horizontal      */
	        0xA1U,             /* Segment Remap (vertical mirror)                 */
	        0xC8U,             /* COM Output Scan Direction (Horizantal Mirror)    */
	        0xDAU, 0x12U,      /* COM Pins Hardware Config برای 128x64      */
	        0x81U, 0xCFU,      /* Contrast Control                         */
	        0xD9U, 0xF1U,      /* Pre-charge Period                        */
	        0xDBU, 0x40U,      /* VCOMH Deselect Level                     */
	        0xA4U,             /* Entire Display ON از RAM (نه همه روشن)    */
	        0xA6U,             /* Normal Display (no Inverted)              */
	        0xAFU              /* Display ON                               */
	    };
	    for (uint32_t i = 1; i < sizeof(init_cmds); i++) {
#if DEBUG
        	char str[10];
        	sprintf(str,"Buffer:%d\r",init_cmds[i]);
        	usart_write_string(str);
#endif
	        if (ssd1306_write_command(init_cmds[i]) != 0) {
#if DEBUG

	        	usart_write_string("SSD1306 Init is faulf\r");
#endif
	            return -1; /* OLED did not respond on I2C bus */
	        }
	    }

	    SSD1306_Clear();
	    return SSD1306_UpdateScreen();
}
void SSD1306_Clear(void)
{
    for (uint32_t i = 0; i < sizeof(s_framebuffer); i++) {
        s_framebuffer[i] = 0U;
    }
}

int SSD1306_UpdateScreen(void)
{
	/* Set the addressing range to the entire page (0..127 columns, 0..7 pages) to return auto-address after writing with
	* Memory Addressing Mode=Horizontal */
    if (ssd1306_write_command(0x21U) != 0) return -1; /* Set Column Address */
    if (ssd1306_write_command(0x00U) != 0) return -1;
    if (ssd1306_write_command(0x7FU) != 0) return -1;

    if (ssd1306_write_command(0x22U) != 0) return -1; /* Set Page Address */
    if (ssd1306_write_command(0x00U) != 0) return -1;
    if (ssd1306_write_command(0x07U) != 0) return -1;

    /* Send the entire Frame Buffer in one I2C transaction: Data control byte once,
    * then 1024 bytes in a row (I2C_WriteBuffer reads from a simple array,
    * so we send the control byte separately first and the rest in a buffer). */
    if (I2C_Start() != 0) return -1;
    if (I2C_SendAddress(SSD1306_I2C_ADDR, 0U) != 0) return -1;
    if (I2C_SendByte(SSD1306_CTRL_DATA) != 0) { I2C_Stop(); return -1; }

    for (uint32_t i = 0; i < sizeof(s_framebuffer); i++) {
        if (I2C_SendByte(s_framebuffer[i]) != 0) {
            I2C_Stop();
            return -1;
        }
    }
    I2C_Stop();

    return 0;
}

void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return; /* Out of page bounds - ignored */
    }

    uint32_t index = (uint32_t)x + ((uint32_t)(y / 8U) * SSD1306_WIDTH);
    uint8_t  bit   = (uint8_t)(1U << (y % 8U));

    if (color == SSD1306_COLOR_WHITE) {
        s_framebuffer[index] |= bit;
    } else {
        s_framebuffer[index] &= (uint8_t)~bit;
    }
}

void SSD1306_DrawChar(uint8_t x, uint8_t y, char c, uint8_t color)
{
    const uint8_t *glyph = font5x7_get(c);

    for (uint8_t col = 0; col < 5U; col++) {
        uint8_t line = glyph[col];
        for (uint8_t row = 0; row < 7U; row++) {
            uint8_t pixel_on = (uint8_t)((line >> row) & 0x01U);
            SSD1306_DrawPixel((uint8_t)(x + col), (uint8_t)(y + row),
                               pixel_on ? color : (uint8_t)(1U - color));
        }
    }
}

void SSD1306_DrawString(uint8_t x, uint8_t y, const char *str, uint8_t color)
{
    uint8_t cursor_x = x;

    while (*str != '\0') {
        SSD1306_DrawChar(cursor_x, y, *str, color);
        cursor_x = (uint8_t)(cursor_x + 6U); /* Character width (5) + 1 pixel spacing */
        str++;

        if (cursor_x > (SSD1306_WIDTH - 5U)) {
            break; /* Reaching the end of the page - prevents overflow */
        }
    }
}
