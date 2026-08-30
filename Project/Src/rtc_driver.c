/*
 * rtc_deriver.c
 *
 *  Created on: Aug 29, 2026
 *      Author: ali Hayati
 */
#include "rtc_driver.h"

#define RTC_STARTUP_TIMEOUT   1000000UL

/* RTCCLK = LSE = 32768Hz -> for a pulse exactly every 1 second:
* PRL = RTCCLK/1Hz - 1 = 32767 = 0x7FFF */
#define RTC_PRELOAD_1HZ        32767UL

static int wait_flag_set(volatile uint32_t *reg,uint32_t mask,uint32_t timeout)
{
	while ((*reg & mask) == 0U)
	{
		if (--timeout == 0U)
		{
			return -1;
		}
	}
	return 0;
}
/* =========================================================================================================
* Safe sequence for writing to RTC registers (required according to RM0008 - RTC section):
* 1) Wait until RTOFF=1 (verify that the previous write is complete)
* 2) CNF=1 (enter configuration mode)
* 3) [caller writes new value to PRLH/L or CNTH/L or ALRH/L]
* 4) CNF=0 (exit; actual write is performed here)
* 5) Wait until RTOFF is 1 again (verify that the write is complete)
* =====================================================================================================*/
static int rtc_enter_config_mod (void)
{
	if (wait_flag_set(&RTC->CRL, RTC_CRL_RTOFF, RTC_STARTUP_TIMEOUT) != 0 )
	{
		return -1;
	}
	RTC->CRL |= RTC_CRL_CNF;
	return 0;
}
static int rtc_exit_config_mod(void)
{
	RTC->CRL &= ~RTC_CRL_CNF;
	return wait_flag_set(&RTC->CRL,RTC_CRL_RTOFF, RTC_STARTUP_TIMEOUT);

}

int RTC_Init(void)
{
	/* 1) Unlock Backup domain writes - Without this bit, any writes to
	* RCC->BDCR or RTC/BKP registers are ignored. */
	PWR->CR |= PWR_CR_DBP;

	RCC->BDCR |= RCC_BDCR_LSEON;
	if (wait_flag_set(&RCC->BDCR,RCC_BDCR_LSERDY,RTC_STARTUP_TIMEOUT) != 0)
	{
		return -1;/* LSE crystal is not installed/emulated or is faulty */
	}

	/* 3) Select LSE as RTC clock source, then activate the RTC block itself.
	* Note: If the compiler gives an undeclared
	* error on the name RCC_BDCR_RTCSEL_LSE, replace it with RCC_BDCR_RTCSEL_0 (both are equivalent to selecting
	* LSE; only the alias is different - exactly the same problem we had with
	* RCC_CFGR_PLLSRC in phase 1). */
	RCC->BDCR &= ~RCC_BDCR_RTCSEL;
	RCC->BDCR |= RCC_BDCR_RTCSEL_LSE;
	RCC->BDCR |= RCC_BDCR_RTCEN;

	/* 4) Wait for the RTC registers (CNT/DIV) to sync with the APB1 domain.
	* According to RM0008: After each Backup domain Reset, we must explicitly set RSF to zero
	* and wait for it to be set again, before any read/write. */
	RTC->CRL &= ~RTC_CRL_RSF;
	if (wait_flag_set(&RTC->CRL, RTC_CRL_RSF, RTC_STARTUP_TIMEOUT) != 0)
	{
		return -1;
	}

	/* 5) Set Prescaler to be exactly 1 second accurate (as per safe sequence above) */
	if (rtc_enter_config_mod() != 0)
	{
		return -1;
	}
	RTC->PRLH = (uint16_t)((RTC_PRELOAD_1HZ >> 16) & 0xFU);
	RTC->PRLL = (uint16_t)(RTC_PRELOAD_1HZ & 0xFFFFU);
	if (rtc_exit_config_mod() != 0)
	{
		return -1;
	}
	return 0;
}

void RTC_SetCounter(uint32_t unix_seconds)
{
	if (rtc_enter_config_mod() != 0)
	{
		return ;
	}
	RTC->CNTH = (uint16_t)((unix_seconds >> 16) & 0xFU);
	RTC->CNTL = (uint16_t)(unix_seconds & 0xFFFFU);
	rtc_exit_config_mod();
}

uint32_t RTC_GetCounter(void)
{
	uint16_t high1, high2,low;
	do
	{
		high1 = RTC->CNTH;
		low = RTC->CNTL;
		high2 = RTC->CNTH;
	}while (high1 != high2);
	return ((uint32_t) high2 << 16) | (uint32_t) low;
}

uint8_t RTC_SecondFlagIsSet(void)
{
    return ((RTC->CRL & RTC_CRL_SECF) != 0U) ? 1U : 0U;
}

void RTC_ClearSecondFlag(void)
{
	/* RTC_CRL status flags (SECF/ALRF/OWF) are cleared only by writing zero;
	* This is independent of the RTOFF/CNF sequence (as these flags are not part of the
	* Configuration state, but are merely hardware event reports). */
    RTC->CRL &= ~RTC_CRL_SECF;
}
