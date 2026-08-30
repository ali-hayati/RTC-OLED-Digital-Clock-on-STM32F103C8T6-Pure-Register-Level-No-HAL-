/*
 * calendar_convert.h
 *
 *  Created on: Aug 29, 2026
 *      Author: ali Hayati
 */

#ifndef CALENDAR_CONVERT_H_
#define CALENDAR_CONVERT_H_

#include <stdint.h>

/* This module intentionally does not use the standard C <time.h>, because in
* bare-metal projects, a full implementation of mktime/localtime usually relies on the operating system's functions
* which are either not available in a non-RTOS/OS environment or add a lot of
* to the binary. The following algorithm is based on Howard
* Hinnant's well-known method for converting the number of days from an epoch to a Gregorian (Proleptic
* Gregorian) date, and correctly calculates the calendar without decimal division. */
typedef struct {
    uint16_t year;    /* مثلاً 2026 */
    uint8_t  month;   /* 1..12 */
    uint8_t  day;     /* 1..31 */
    uint8_t  hour;    /* 0..23 */
    uint8_t  minute;  /* 0..59 */
    uint8_t  second;  /* 0..59 */
} calendar_time_t;

/**
* @brief Converts raw RTC counter (seconds since Unix epoch: 1970-01-01 00:00:00)
* to Gregorian date and time.
* @param unix_seconds The value read from RTC_GetCounter()
* @param out A pointer to the output struct
*/
void calendar_from_unix(uint32_t unix_seconds, calendar_time_t *out);

#endif /* CALENDAR_CONVERT_H_ */
