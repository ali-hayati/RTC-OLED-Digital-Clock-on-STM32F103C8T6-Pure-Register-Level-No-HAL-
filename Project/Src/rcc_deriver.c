/*
 * rcc_deriver.c
 *
 *  Created on: Aug 29, 2026
 *      Author: ali Hayati
 */
#include "rcc_driver.h"

#define RCC_STARTUP_TIMEOUT   100000UL
/**
* @brief Wait for a specific bit in a register to be set, with a time limit
* @return 0 if the bit is set, -1 if a timeout occurs
*/
static int wait_flag_set(volatile uint32_t *reg, uint32_t mask, uint32_t timeout)
{
    while ((*reg & mask) == 0U) {
        if (--timeout == 0U) {
            return -1;
        }
    }
    return 0;
}
int RCC_SystemClockConfig(void)
{
	/* 1) Turn on the HSE (external 8MHz crystal) and wait for it to be ready */
    RCC->CR |= RCC_CR_HSEON;
    if (wait_flag_set(&RCC->CR, RCC_CR_HSERDY, RCC_STARTUP_TIMEOUT) != 0) {
        return -1;
    }
    /* 2) Adjust Flash Latency before increasing frequency (required by RM0008) */
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= (FLASH_ACR_LATENCY_2 | FLASH_ACR_PRFTBE);

    /* 3) Prescale the buses before activating the PLL:
     *    AHB = SYSCLK/1 , APB1 = SYSCLK/2 , APB2 = SYSCLK/1 */
    RCC->CFGR &= ~RCC_CFGR_HPRE;
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

    RCC->CFGR &= ~RCC_CFGR_PPRE1;
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

    RCC->CFGR &= ~RCC_CFGR_PPRE2;
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;

    /* 4) PLL source = HSE without division, PLL multiplier = x9 -> 8MHz * 9 = 72MHz */
    RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE);
    RCC->CFGR |= RCC_CFGR_PLLSRC;

    RCC->CFGR &= ~RCC_CFGR_PLLMULL;
    RCC->CFGR |= RCC_CFGR_PLLMULL9;

    /* 5) Turn on the PLL and wait for it to lock */
    RCC->CR |= RCC_CR_PLLON;
    if (wait_flag_set(&RCC->CR, RCC_CR_PLLRDY, RCC_STARTUP_TIMEOUT) != 0) {
        return -1;
    }
    /* 6) Switch SYSCLK to PLL and wait for hardware confirmation (SWS) */
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {
    	/* In practice this switch is done almost immediately; this loop is only
    	* for safe synchronization with the hardware. */
    }
    return 0;
}

void RCC_EnableGPIOClocks(void)
{
    RCC->APB2ENR |= (RCC_APB2ENR_AFIOEN |
                      RCC_APB2ENR_IOPAEN |
                      RCC_APB2ENR_IOPBEN |
                      RCC_APB2ENR_IOPCEN);
}

void RCC_EnablePWR_BKP_Clocks(void)
{
	/* Mandatory prerequisite for accessing RTC/Backup registers in phase 2 */
    RCC->APB1ENR |= (RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN);
}

void RCC_EnableI2C1Clock(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
}
