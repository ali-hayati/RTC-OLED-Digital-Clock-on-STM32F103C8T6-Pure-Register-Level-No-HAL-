/*
 * I2C_driver.h
 *
 *  Created on: Aug 29, 2026
 *      Author: ali Hayati
 */

#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include "stm32f103xb.h"

/* I2C1 on default pins (no Remap): PB6 = SCL, PB7 = SDA
* Selectable speed: Standard Mode 100kHz (more compatibility, easier testing in Proteus) */
#define I2C1_SCL_PORT   GPIOB
#define I2C1_SCL_PIN    6U
#define I2C1_SDA_PORT   GPIOB
#define I2C1_SDA_PIN    7U

#define I2C1_FREQ_HZ    100000UL

/**
* @brief Full I2C1 setup: Clock, AF Open-Drain pins, and registers
* FREQ/CCR/TRISE based on APB1=36MHz to achieve 100kHz precision
*/
void I2C_Init(void);

/**
* @brief Send START condition and wait for SB flag
* @return 0 on success, -1 on timeout
*/
int I2C_Start(void);

/**
* @brief Send STOP condition
*/
void I2C_Stop(void);

/**
* @brief Send 7-bit address + read/write bit, and correctly clear ADDR flag
* @param addr7 7-bit device address (no shift)
* @param read 0 = write, 1 = read
* @return 0 on success (ACK received), -1 on NACK or timeout
*/
int I2C_SendAddress(uint8_t addr7, uint8_t read);

/**
* @brief Send one byte and wait for TXE
* @return 0 on success, -1 on timeout
*/
int I2C_SendByte(uint8_t data);

/**
* @brief High-level function: A complete write transaction (START + address + buffer + STOP)
* This is the function that SSD1306 uses to send commands/data.
* @return 0 on success, -1 on error (OLED not connected, NACK, etc.)
*/
int I2C_WriteBuffer(uint8_t addr7, const uint8_t *data, uint16_t len);

#endif /* I2C_DRIVER_H */
