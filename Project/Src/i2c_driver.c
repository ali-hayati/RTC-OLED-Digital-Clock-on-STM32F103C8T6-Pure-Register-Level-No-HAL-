/*
 * I2C_deriver.c
 *
 *  Created on: Aug 29, 2026
 *      Author: ali Hayati
 */
#include "i2c_driver.h"
#include  "rtc_driver.h"
#include  "gpio_drive.h"
#include "rcc_driver.h"
#include "Uart.h"
#include "stdio.h"
#define I2C_TIMEOUT   200000UL
#define DEBUG 	0
/**
* @brief Wait for a flag to be set in SR1, with a time limit
*/
static int wait_sr1_flag(uint32_t mask, uint32_t timeout)
{
    while ((I2C1->SR1 & mask) == 0U) {
        if (--timeout == 0U) {
            return -1;
        }
    }
    return 0;
}

void I2C_Init(void)
{
	RCC_EnableI2C1Clock();

	/* SCL/SDA pins must be Alternate-Function Open-Drain; the I2C bus
	* is held high by a pull-up (external on most OLED modules, or internal on STM32)
	* and the pin is only connected to GND, never Push-Pull. */
    GPIO_ConfigAFOpenDrain(I2C1_SCL_PORT, I2C1_SCL_PIN, 50U);
    GPIO_ConfigAFOpenDrain(I2C1_SDA_PORT, I2C1_SDA_PIN, 50U);

    /* Soft reset peripheral before configuration (as recommended by RM0008) */
    I2C1->CR1 |= I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    /* CR2.FREQ should be the APB1 frequency in MHz; since APB1=36MHz: */
    I2C1->CR2 &= ~I2C_CR2_FREQ;
    I2C1->CR2 |= (36UL & 0x3FUL);

    /* CCR برای Standard Mode (100kHz، Duty=0 یعنی Tlow=Thigh):
       *   CCR = Fpclk1 / (2 * Fi2c) = 36,000,000 / (2*100,000) = 180 */
    uint32_t ccr_value = 36000000UL / (2UL * I2C1_FREQ_HZ);
    I2C1->CCR &= ~I2C_CCR_CCR;
    I2C1->CCR |= (ccr_value & 0x0FFFUL);/* Standard Mode: F/S=0, DUTY=0 */

    /* TRISE (maximum allowed rise time 1000ns in Standard Mode):
    * TRISE = (Fpclk1_MHz * 1000ns / 1000) + 1 = 36 + 1 = 37 */
    I2C1->TRISE = (36UL + 1UL) & 0x3FUL;

    I2C1->CR1 |= I2C_CR1_PE;/* Activate peripheral */
}

int I2C_Start(void)
{
    I2C1->CR1 |= I2C_CR1_START;
    return wait_sr1_flag(I2C_SR1_SB, I2C_TIMEOUT);
}

void I2C_Stop(void)
{
    I2C1->CR1 |= I2C_CR1_STOP;
}
int I2C_SendAddress(uint8_t addr7, uint8_t read)
{
    uint32_t timeout = I2C_TIMEOUT;

    I2C1->DR = (uint32_t)((addr7 << 1) | (read ? 1U : 0U));
    while (1)
    {
#if DEBUG
    uint32_t read_reg1,read_reg2;
    read_reg1 = I2C1->SR1;
    read_reg2 = I2C1->SR2;
#endif
    	if((I2C1->SR1 & I2C_SR1_ADDR) != 0)
    	{
#if DEBUG
    		usart_write_string("Address Flag is ok\r");
#endif
    		/* Clearing ADDR according to the datasheet: by sequentially reading SR1 then SR2 */
            (void)I2C1->SR1;
            (void)I2C1->SR2;
            return 0;
    	}
    	if ((I2C1->SR1 & I2C_SR1_AF) != 0)
    	{
            I2C1->SR1 &= ~I2C_SR1_AF; /* Clear NACK flag */
            I2C_Stop();
            return -1;
    	}
        if (--timeout == 0U) {
            I2C_Stop();
#if DEBUG
            for (uint32_t i=0; i<1000000;i++);
            uint32_t read_reg3;
            read_reg3= I2C1->SR2;
            char str[10];
            sprintf(str,"SR1:%ld\r",read_reg1);
            usart_write_string(str);
            sprintf(str,"SR2:%ld\r",read_reg2);
            usart_write_string(str);
            sprintf(str,"SR2:%ld\r",read_reg3);
            usart_write_string(str);
#endif
            return -1;
        }
    }
}
int I2C_SendByte(uint8_t data)
{
	uint32_t timeout = I2C_TIMEOUT;
#if DEBUG
	char str[10];
	sprintf(str,"data:%d\r",data);
	usart_write_string(str);
#endif
	I2C1->DR = data;

	while ((I2C1->SR1 & I2C_SR1_TXE) == 0U)
	{
		if((I2C1->SR1 & I2C_SR1_AF) != 0U)
		{
            I2C1->SR1 &= ~I2C_SR1_AF;
            return -1;

		}
        if (--timeout == 0U) {
            return -1;
        }
	}
	I2C1->DR = data;
	return 0;
}

int I2C_WriteBuffer(uint8_t addr7, const uint8_t *data, uint16_t len)
{
#if DEBUG
	char str[10];
	sprintf(str,"Len:%d\r",len);
	usart_write_string(str);
#endif
    if (I2C_Start() != 0) {
        return -1;
    }
    if (I2C_SendAddress(addr7, 0) != 0) {
        return -1;
    }
    for (uint16_t i = 0; i < len; i++) {
        if (I2C_SendByte(data[i]) != 0) {
            I2C_Stop();
            return -1;
        }
    }
    I2C_Stop();
    return 0;
}
