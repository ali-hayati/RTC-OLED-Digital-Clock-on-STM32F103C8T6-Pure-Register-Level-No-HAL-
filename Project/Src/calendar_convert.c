/*
 * calendar_convert.c
 *
 *  Created on: Aug 29, 2026
 *      Author: ali Hayati
 */
#include "calendar_convert.h"

void calendar_from_unix(uint32_t unix_seconds, calendar_time_t *out)
{
	/* --- Separate the hour/minute/second part from the date part --- */
    uint32_t days = unix_seconds / 86400UL;
    uint32_t rem  = unix_seconds % 86400UL;

    out->hour   = (uint8_t)(rem / 3600UL);
    rem        %= 3600UL;
    out->minute = (uint8_t)(rem / 60UL);
    out->second = (uint8_t)(rem % 60UL);

    /* --- civil_from_days algorithm (Howard Hinnant) ---
    * Input: number of days since 1970-01-01 (days)
    * Output: Gregorian year/month/day
    * By shifting the calendar origin to 0000-03-01 (instead of 01-01),
    * this algorithm turns leap year calculations into simple integer division - no need for
    * a loop or table of the number of days in each month. */
    int32_t  z   = (int32_t)days + 719468;                 /* Origin shift */
    int32_t  era = (z >= 0 ? z : z - 146096) / 146097;      /* Each era = 400 years */
    uint32_t doe = (uint32_t)(z - era * 146097);            /* day-of-era: [0,146096] */
    uint32_t yoe = (doe - doe / 1460U + doe / 36524U - doe / 146096U) / 365U; /* [0,399] */
    int32_t  y   = (int32_t)yoe + era * 400;
    uint32_t doy = doe - (365U * yoe + yoe / 4U - yoe / 100U);                /* [0,365] */
    uint32_t mp  = (5U * doy + 2U) / 153U;                  /* Shifted month: [0,11] */
    uint32_t d   = doy - (153U * mp + 2U) / 5U + 1U;       /* Day of the month: [1,31] */
    uint32_t m   = mp + ((mp < 10U) ? 3U : (uint32_t)-9);   /* Actual month: [1,12] */

    y += (int32_t)(m <= 2U);   /* If it was January/February, it belongs to the next year era */

    out->year  = (uint16_t)y;
    out->month = (uint8_t)m;
    out->day   = (uint8_t)d;
}

