# Timing Analysis

This document shows every numerical calculation behind the peripheral configurations in this project, with formula and final value — the goal is that no magic number appears in the code without a clear mathematical justification.

---

## 1. Main Clock Tree (PLL)

**Input:** HSE crystal = 8MHz
**Target:** SYSCLK = 72MHz (maximum allowed for STM32F103)

```
SYSCLK = HSE × PLLMULL
72MHz  = 8MHz × 9   ->  PLLMULL = 9  (register code: 0111)
```

**Bus prescalers:**

| Bus | Formula | Value | Constraint |
|---|---|---|---|
| AHB | SYSCLK / 1 | 72MHz | Max 72MHz ✓ |
| APB1 | AHB / 2 | 36MHz | **Max allowed is 36MHz** — dividing by 2 is mandatory |
| APB2 | AHB / 1 | 72MHz | Max 72MHz ✓ |

**Flash Latency:** Per the RM0008 table, `48MHz < SYSCLK ≤ 72MHz` requires `LATENCY = 2 wait states`, otherwise the CPU reads Flash memory faster than it can physically respond, causing undefined behavior / random crashes.

---

## 2. RTC Prescaler (1-Second Resolution)

**Input:** LSE = 32.768 kHz (an industry standard, exactly 2^15 Hz — which is precisely why it's used for timekeeping)

```
TR_CLK = RTCCLK / (PRL + 1)
1 Hz   = 32768 / (PRL + 1)
PRL    = 32768 - 1 = 32767 = 0x7FFF
```

**Timing accuracy:** Typical low-cost 32.768kHz crystals have a tolerance of around ±20ppm, giving a cumulative error of roughly:
```
Daily drift ≈ 20 × 10⁻⁶ × 86400 seconds ≈ ±1.7 seconds per day
```
For higher accuracy, a temperature-compensated crystal (TCXO) would be a better choice — outside the scope of this project.

---

## 3. I2C1 — Standard Mode (100kHz)

**Input:** APB1 = 36MHz (from the calculation in Section 1)

### FREQ Field (CR2 register)
```
FREQ = APB1 frequency in MHz = 36
```

### CCR Field (CCR register) — determines the actual SCL frequency
In Standard Mode with `DUTY=0`, high and low times are equal:
```
CCR = Fpclk1 / (2 × Fi2c)
CCR = 36,000,000 / (2 × 100,000)
CCR = 180
```

### TRISE Field — maximum allowed rise time
The I2C standard / SSD1306 datasheet defines the maximum rise time in Standard Mode as 1000ns:
```
TRISE = (Fpclk1_MHz × Trise_max_ns / 1000ns) + 1
TRISE = (36 × 1000 / 1000) + 1
TRISE = 37
```

> **Why 100kHz instead of 400kHz (Fast Mode)?** Standard Mode was chosen to ensure compatibility with Proteus simulation and most low-cost OLED modules. To increase the frame rate (e.g., for animation), `I2C1_FREQ_HZ` could be changed to 400000 and CCR/TRISE recalculated (using Fast Mode's different formulas) to increase speed.

---

## 4. SysTick — 1 Millisecond Tick

**Input:** Core clock (AHB) = 72MHz

```
LOAD = (SYSCLK_Hz / 1000) - 1
LOAD = (72,000,000 / 1000) - 1
LOAD = 71999
```

Each time the counter reaches zero from `LOAD`, exactly 1 millisecond has elapsed, and the `COUNTFLAG` flag (or, in interrupt mode, the ISR itself) is triggered.

---

## 5. Button Debounce and Long-Press Detection

### Why 30 milliseconds for debounce?
Mechanical contact bounce in typical tact switches usually lasts between 5 and 20 milliseconds. 30ms provides a reasonable safety margin — not so short that noise slips through, and not so long that fast genuine button presses get missed.

### Why an 800 millisecond threshold for long-press?
This is an empirical/UX choice, not a hardware constraint:
- Below ~500ms: risk of misclassifying a slightly slow normal press as "long"
- Above ~1200ms: the button starts to feel slow/unresponsive to the user

The value 800ms is configurable in this project via `BUTTON_LONG_PRESS_MS` (`inc/button_driver.h`).

---

## Summary of All Calculated Values

| Parameter | Formula | Final Value |
|---|---|---|
| `PLLMULL` | 72MHz / 8MHz | 9 (code 0111) |
| `FLASH_ACR.LATENCY` | RM0008 table for 72MHz | 2 |
| `RTC_PRL` | 32768 - 1 | 0x7FFF (32767) |
| `I2C1_CR2.FREQ` | APB1 in MHz | 36 |
| `I2C1_CCR` | 36MHz / (2×100kHz) | 180 |
| `I2C1_TRISE` | 36 + 1 | 37 |
| `SysTick_LOAD` | 72MHz / 1000 - 1 | 71999 |
| `BUTTON_DEBOUNCE_MS` | Empirical | 30 |
| `BUTTON_LONG_PRESS_MS` | Empirical/UX | 800 |
