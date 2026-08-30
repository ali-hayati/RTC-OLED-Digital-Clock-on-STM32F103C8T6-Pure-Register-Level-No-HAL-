#include "stm32f103xb.h"
#include "rcc_driver.h"
#include "gpio_drive.h"
#include "rtc_driver.h"
#include "calendar_convert.h"
#include "i2c_driver.h"
#include "ssd1306_driver.h"
#include "systick_deriver.h"
#include "button_driver.h"

/* Blue Pill onboard LED: PC13, Active-LOW (0 = on, 1 = off) */
#define LED_PORT   GPIOC
#define LED_PIN    13U

/* Initial test value: 2024-01-01 00:00:00 UTC (Unix Timestamp = 1704067200) */
#define TEST_INITIAL_UNIX_TIME   1704067200UL

volatile calendar_time_t g_now; /* Visible in Live Expressions even without OLED */

/* --- state machine Manual clock setting with one button ---
* RUN : Normal display of live clock
* EDIT_HOUR : Short press = +1 hour, long press = go to EDIT_MINUTE
* EDIT_MINUTE : Short press = +1 minute, long press = save to RTC and return to RUN
*/
typedef enum {
    APP_STATE_RUN = 0,
    APP_STATE_EDIT_HOUR,
    APP_STATE_EDIT_MINUTE
} app_state_t;

static app_state_t s_app_state = APP_STATE_RUN;
static uint8_t     s_edit_hour   = 0U;
static uint8_t     s_edit_minute = 0U;
/* "Today's" start time (second 00:00:00 of the same day) - calculated once at the moment of entering the
* edit mode and kept constant, until the user saves
* the same base is used to create the final Timestamp. */
static uint32_t    s_edit_day_start_unix = 0UL;

/* --- Distinctive error blink patterns, for quick detection of a failed stage without a debugger --- */
static void error_blink_forever(uint32_t half_period_loops)
{
    while (1) {
        GPIO_TogglePin(LED_PORT, LED_PIN);
        for (volatile uint32_t i = 0; i < half_period_loops; i++) { }
    }
}

static void format_2digit(uint8_t value, char *dst)
{
    dst[0] = (char)('0' + (value / 10U));
    dst[1] = (char)('0' + (value % 10U));
}

static void format_4digit(uint16_t value, char *dst)
{
    dst[0] = (char)('0' + (value / 1000U));
    dst[1] = (char)('0' + ((value / 100U) % 10U));
    dst[2] = (char)('0' + ((value / 10U) % 10U));
    dst[3] = (char)('0' + (value % 10U));
}

/**
* @brief Draw a short underline (Cursor) under two specified digits, to indicate
* which field (hour/minute) is being edited - without the need for
* the letters of the alphabet in the font (which are not currently implemented)
*/
static void draw_field_cursor(uint8_t base_x, uint8_t char_index)
{
    uint8_t x_start = (uint8_t)(base_x + (char_index * 6U));
    uint8_t y        = 48U; /* Just below the seventh row of glyphs (which extends to y+7) */

    for (uint8_t i = 0; i < 11U; i++) { /* Width of 2 digits + space between them */
        SSD1306_DrawPixel((uint8_t)(x_start + i), y, SSD1306_COLOR_WHITE);
    }
}

/**
* @brief Full page rendering: date, time, and (if edit mode is enabled)
* Underline indicator under the field being changed
*/
static void render_screen(const calendar_time_t *displayed_time)
{
    char date_str[11] = "0000-00-00";
    format_4digit(displayed_time->year, &date_str[0]);
    format_2digit(displayed_time->month, &date_str[5]);
    format_2digit(displayed_time->day, &date_str[8]);

    char time_str[9] = "00:00:00";
    format_2digit(displayed_time->hour, &time_str[0]);
    format_2digit(displayed_time->minute, &time_str[3]);
    format_2digit(displayed_time->second, &time_str[6]);

    SSD1306_Clear();
    SSD1306_DrawString(0U, 0U, "STM32 BareMetal RTC", SSD1306_COLOR_WHITE);
    SSD1306_DrawString(20U, 24U, date_str, SSD1306_COLOR_WHITE);
    SSD1306_DrawString(20U, 40U, time_str, SSD1306_COLOR_WHITE);

    if (s_app_state == APP_STATE_EDIT_HOUR) {
        draw_field_cursor(20U, 0U); /* Below two digits of the hour (character index 0 and 1) */
    } else if (s_app_state == APP_STATE_EDIT_MINUTE) {
        draw_field_cursor(20U, 3U); /* Below two digits of the minute (after "HH:") */
    }

    SSD1306_UpdateScreen();
}

/**
* @brief Process a button event based on the current state; all manual setup logic
* The clock is centered here
*/
static void handle_button_event(button_event_t event)
{
    if (event == BUTTON_EVENT_NONE) {
        return;
    }

    if (s_app_state == APP_STATE_RUN) {
        if (event == BUTTON_EVENT_LONG_PRESS) {
        	/* Enter edit mode: Base the "start of day" once from the current RTC time
        	* to keep it constant during editing */
            uint32_t raw = RTC_GetCounter();
            calendar_time_t t;
            calendar_from_unix(raw, &t);

            uint32_t seconds_since_midnight = (uint32_t)t.hour * 3600UL +
                                               (uint32_t)t.minute * 60UL +
                                               (uint32_t)t.second;
            s_edit_day_start_unix = raw - seconds_since_midnight;
            s_edit_hour   = t.hour;
            s_edit_minute = t.minute;
            s_app_state   = APP_STATE_EDIT_HOUR;
        }
        /* Short press in RUN mode is intentionally ineffective */

    } else if (s_app_state == APP_STATE_EDIT_HOUR) {
        if (event == BUTTON_EVENT_SHORT_PRESS) {
            s_edit_hour = (uint8_t)((s_edit_hour + 1U) % 24U);
        } else { /* LONG_PRESS */
            s_app_state = APP_STATE_EDIT_MINUTE;
        }

    } else { /* APP_STATE_EDIT_MINUTE */
        if (event == BUTTON_EVENT_SHORT_PRESS) {
            s_edit_minute = (uint8_t)((s_edit_minute + 1U) % 60U);
        } else { /* LONG_PRESS -> Save and Exit */
            uint32_t new_unix = s_edit_day_start_unix +
                                 ((uint32_t)s_edit_hour * 3600UL) +
                                 ((uint32_t)s_edit_minute * 60UL);
            RTC_SetCounter(new_unix); /* ثانیه از صفر شروع می‌شود */
            s_app_state = APP_STATE_RUN;
        }
    }

    /* Instant rendering after each button event, so that the UI responds without delay until the next RTC * second is sensed */
    calendar_time_t displayed;
    if (s_app_state == APP_STATE_RUN) {
        calendar_from_unix(RTC_GetCounter(), &displayed);
    } else {
        displayed.year = 0U; displayed.month = 0U; displayed.day = 0U; /* نمایش نشده */
        displayed.hour = s_edit_hour;
        displayed.minute = s_edit_minute;
        displayed.second = 0U;
    }
    render_screen(&displayed);
}

int main(void)
{
	/* --- Phase 1: Main Clock Tree --- */
    if (RCC_SystemClockConfig() != 0) {
        RCC_EnableGPIOClocks();
        GPIO_ConfigOutputPP(LED_PORT, LED_PIN, 2U);
        error_blink_forever(300000UL);
    }

    RCC_EnableGPIOClocks();
    GPIO_ConfigOutputPP(LED_PORT, LED_PIN, 2U);
    SysTick_InitTick(); /* Button Debounce Prerequisite; must be before Button_Init */

    /* --- Phase 2: RTC on LSE --- */
    RCC_EnablePWR_BKP_Clocks();
    if (RTC_Init() != 0) {
        error_blink_forever(50000UL);
    }
    RTC_SetCounter(TEST_INITIAL_UNIX_TIME);

    /* --- Phase 3: I2C + OLED --- */
    I2C_Init();
    if (SSD1306_Init() != 0) {
        error_blink_forever(700000UL);
    }

    /* --- Phase 4: Manual clock setting button --- */
    Button_Init();

    while (1) {
        button_event_t event = Button_GetEvent();
        if (event != BUTTON_EVENT_NONE) {
            handle_button_event(event);
        }

        if ((s_app_state == APP_STATE_RUN) && (RTC_SecondFlagIsSet() != 0U)) {
            RTC_ClearSecondFlag();
            GPIO_TogglePin(LED_PORT, LED_PIN);

            calendar_time_t local_now;
            calendar_from_unix(RTC_GetCounter(), &local_now);
            g_now = local_now;

            render_screen(&local_now);
        }
    }
}
