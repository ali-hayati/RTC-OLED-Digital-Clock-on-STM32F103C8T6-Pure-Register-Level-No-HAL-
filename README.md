# RTC + OLED Digital Clock on STM32F103C8T6 — Pure Register-Level (No HAL)

A complete digital clock built with the **internal RTC**, an **SSD1306 OLED display (128×64)**, and **single-button manual time-setting**, implemented entirely through direct register access on the STM32F103C8T6 microcontroller — no STM32 HAL, no CMSIS Driver layer, and no pre-built graphics library.

## Why Register-Level?

Most similar projects on GitHub rely on `HAL_RTC_Init()` and ready-made OLED libraries (e.g. u8g2), which hide what's actually happening in hardware. This project takes the opposite approach: showing **exactly what happens at the register level** — from clock-tree bring-up down to the bit-by-bit I2C protocol sequence. This style is better suited both for deep learning and for demonstrating hands-on technical mastery (e.g., for a resume/portfolio).

## Features

- Fully register-level clock tree setup: `HSE(8MHz) → PLL×9 → 72MHz`
- Internal RTC running on a `32.768kHz LSE` crystal, independent of the main system clock
- A hand-written algorithm converting Unix timestamps to Gregorian calendar dates (no `<time.h>`)
- I2C driver written from scratch (Start/Stop/Address/Byte, with timeouts at every stage)
- SSD1306 driver written from scratch, with a manual frame buffer and a custom 5×7 font
- Manual time setting via a **single button** (EXTI + real time-based debounce, short/long press detection)
- Three distinct LED blink patterns for fast field debugging without a debugger (HSE/PLL failure, LSE failure, OLED failure)

## Required Hardware

| Component | Model |
|---|---|
| Microcontroller | STM32F103C8T6 (Blue Pill) |
| Display | 128×64 OLED with SSD1306 driver (I2C interface) |
| RTC crystal | 32.768kHz (on OSC32_IN/OSC32_OUT pins) |
| Button | Standard tact switch |
| Main crystal | 8MHz (HSE) |

## Wiring

| Signal | STM32 Pin | Note |
|---|---|---|
| Status LED | PC13 | Onboard Blue Pill LED, active-low |
| RTC — OSC32_IN | PC14 | 32.768kHz crystal |
| RTC — OSC32_OUT | PC15 | 32.768kHz crystal |
| I2C1 — SCL | PB6 | Open-drain + pull-up (usually included on OLED module, or external 4.7kΩ) |
| I2C1 — SDA | PB7 | Open-drain + pull-up |
| Time-set button | PA0 | One leg to GND; internal pull-up is enabled, no external resistor needed |

## Project Structure

```
.
├── inc/                    Headers for all drivers
├── src/                    Driver implementations + main.c
├── startup/                Vector table and Reset_Handler (assembly)
├── linker/                 Linker script (64K Flash / 20K RAM)
├── docs/                   Technical documentation (this folder)
└── Makefile                Builds with arm-none-eabi-gcc, no IDE required
```

### Module Map

| Module | Responsibility |
|---|---|
| `rcc_driver` | Clock tree: HSE → PLL → 72MHz |
| `gpio_driver` | Pin configuration: push-pull, AF open-drain, input pull-up |
| `systick_driver` | Interrupt-driven millisecond tick timer (basis for debounce and non-blocking delays) |
| `rtc_driver` | RTC setup on LSE + safe read/write of counter and alarm registers |
| `calendar_convert` | Unix timestamp ↔ Gregorian date conversion (Howard Hinnant's algorithm) |
| `i2c_driver` | Low-level I2C1: Start/Stop/Address/Byte |
| `ssd1306_driver` | OLED init + frame buffer + pixel/text drawing |
| `font` | 5×7 bitmap font for required digits and symbols |
| `button_driver` | EXTI0 + debounce + short/long press detection |
| `main.c` | Integrates all modules + time-setting state machine |

## Building

Using `arm-none-eabi-gcc`, no IDE required:

```bash
sudo apt install gcc-arm-none-eabi stlink-tools   # if not already installed
make            # builds firmware.elf and firmware.bin
make flash      # flashes via ST-Link
```

> If you're using **STM32CubeIDE**, simply copy the files from `inc/` and `src/` into your project's `Inc/` and `Src/` folders; you don't need this repo's Makefile/linker script/startup file, since CubeIDE generates its own.

## Usage

On power-up, the OLED displays a live date and time (initial test value: `2024-01-01 00:00:00`).

**Manual time setting:**
1. **Long-press** the button (≥800ms) → enters hour-setting mode (an underline appears beneath the hour digits)
2. **Short-press** repeatedly to increment the hour value
3. **Long-press again** → moves to minute-setting mode
4. Short-press to adjust the minute value
5. Final **long-press** → the value is written to the RTC and the display returns to normal

## Quick Troubleshooting (No Debugger Needed)

| LED Blink Pattern | Meaning |
|---|---|
| Medium tempo | HSE or PLL failed to start |
| Very fast | LSE crystal failed to start |
| Slow | OLED did not respond on the I2C bus (check address or wiring) |
| Normal 1-second blink | Everything is working correctly |

## Known Limitations & Future Work

- The font currently supports only digits and a few symbols; letters (needed for displaying weekday names) are not yet implemented.
- The initial time is hardcoded; adding full date-setting (not just hour/minute) is a natural next step.
- Polling the RTC's second flag could be replaced with the `RTC_IRQn` interrupt, allowing the CPU to sleep between ticks (power optimization).
- The RTC value is lost on power loss; connecting a coin-cell backup battery to the VBAT pin solves this.

## Technical References

- RM0008 — STM32F101/102/103/105/107 Reference Manual (RCC, RTC, GPIO, I2C, EXTI chapters)
- SSD1306 Datasheet (Solomon Systech)
- Register-by-register details: [`docs/register_map.md`](Docs/register_map.md)
- Frequency and timing calculations: [`docs/timing_analysis.md`](Docs/timing_analysis.md)

## License

MIT — free to use, modify, and distribute.
