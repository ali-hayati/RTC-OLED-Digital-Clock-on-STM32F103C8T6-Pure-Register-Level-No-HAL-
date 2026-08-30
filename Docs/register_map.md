# Register Map

This document lists every hardware register used in the project, organized by peripheral. Primary reference for all values: **RM0008** (STM32F101/102/103/105/107 Reference Manual).

> Note: this project uses ST's official CMSIS header, so macro names exactly match those defined in `stm32f1xx.h` — not arbitrary simplified names.

---

## 1. RCC — Reset and Clock Control

| Register | Base Address | Bits Used | Role in the Project |
|---|---|---|---|
| `RCC_CR` | `0x40021000` | `HSEON`, `HSERDY`, `PLLON`, `PLLRDY` | Turning on HSE and the PLL |
| `RCC_CFGR` | `0x40021004` | `SW`, `SWS`, `HPRE`, `PPRE1`, `PPRE2`, `PLLSRC`, `PLLXTPRE`, `PLLMULL` | Selecting the PLL source and bus prescalers |
| `RCC_APB2ENR` | `0x40021018` | `AFIOEN`, `IOPAEN`, `IOPBEN`, `IOPCEN` | GPIO and AFIO clock enable |
| `RCC_APB1ENR` | `0x4002101C` | `PWREN`, `BKPEN`, `I2C1EN` | PWR/Backup clock (for RTC) and I2C1 clock |
| `RCC_BDCR` | `0x40021020` | `LSEON`, `LSERDY`, `RTCSEL`, `RTCEN` | LSE startup and RTC clock source selection |

**Key clock tree formula:**
```
SYSCLK = HSE × PLLMULL = 8MHz × 9 = 72MHz
AHB    = SYSCLK / HPRE  = 72MHz / 1 = 72MHz
APB1   = AHB / PPRE1    = 72MHz / 2 = 36MHz   (maximum allowed per datasheet)
APB2   = AHB / PPRE2    = 72MHz / 1 = 72MHz
```

---

## 2. FLASH — Flash Access Control

| Register | Base Address | Bits Used | Role in the Project |
|---|---|---|---|
| `FLASH_ACR` | `0x40022000` | `LATENCY`, `PRFTBE` | Setting 2 wait states (required above 48MHz SYSCLK) and enabling the prefetch buffer |

---

## 3. GPIO — General Purpose I/O

| Register | Role |
|---|---|
| `GPIOx_CRL` / `GPIOx_CRH` | Per-pin configuration (4 bits per pin: `MODE`+`CNF`) |
| `GPIOx_ODR` | Current output value / pull direction select (when CNF=Input) |
| `GPIOx_IDR` | Reading the current input level of a pin |
| `GPIOx_BSRR` / `GPIOx_BRR` | Atomic set/reset of a single pin, without read-modify-write |

### Pin Configuration Modes Used in This Project

| Mode | CNF value | MODE value | Used for |
|---|---|---|---|
| Output Push-Pull | `00` | speed (01/10/11) | LED (PC13) |
| AF Open-Drain | `11` | speed | I2C1 SCL/SDA (PB6/PB7) |
| Input Pull-Up | `10` + `ODR=1` | `00` | Button (PA0) |

---

## 4. RTC — Real-Time Clock

| Register | Base Address | Bits Used | Role in the Project |
|---|---|---|---|
| `RTC_CRH` | `0x40002808` | `SECIE`, `ALRIE`, `OWIE` | Interrupt enables (currently unused - polling is used instead) |
| `RTC_CRL` | `0x4000280C` | `RTOFF`, `CNF`, `RSF`, `OWF`, `ALRF`, `SECF` | Write-sequence control + status flags |
| `RTC_PRLH/L` | `0x40002810`/`0x4000281C` | 20-bit prescaler | Setting 1-second resolution: `PRL = 32767 (0x7FFF)` |
| `RTC_DIVH/L` | `0x40002814`/`0x40002818` | 20 bits (read-only) | Live prescaler countdown value (for debugging) |
| `RTC_CNTH/L` | `0x40002820`/`0x40002824` | 32 bits | Main seconds counter (Unix-like) |
| `RTC_ALRH/L` | `0x40002828`/`0x4000282C` | 32 bits | Alarm compare value |

**Safe write sequence (for PRL/CNT/ALR), per RM0008:**
```
1) Wait until RTOFF=1
2) Set CNF=1  (enter configuration mode)
3) Write the new value
4) Clear CNF=0  (exit - the actual write happens here)
5) Wait until RTOFF becomes 1 again
```

---

## 5. PWR — Power Control

| Register | Base Address | Bits Used | Role in the Project |
|---|---|---|---|
| `PWR_CR` | `0x40007000` | `DBP` (Disable Backup Protection) | Unlocking write access to `RCC_BDCR` and the RTC registers |

---

## 6. I2C1

| Register | Base Address | Bits Used | Role in the Project |
|---|---|---|---|
| `I2C1_CR1` | `0x40005400` | `PE`, `START`, `STOP`, `SWRST` | Peripheral enable, generating Start/Stop conditions |
| `I2C1_CR2` | +`0x04` | `FREQ` | APB1 frequency in MHz (36) |
| `I2C1_DR` | +`0x10` | 8-bit data | Sending/receiving a byte |
| `I2C1_SR1` | +`0x14` | `SB`, `ADDR`, `TXE`, `AF` | Transaction status flags |
| `I2C1_SR2` | +`0x18` | — | Only used to clear `ADDR` (must be read right after SR1) |
| `I2C1_CCR` | +`0x1C` | 12 bits | Sets the SCL frequency (`CCR=180` for 100kHz) |
| `I2C1_TRISE` | +`0x20` | 6 bits | Maximum allowed rise time (`TRISE=37` for Standard Mode) |

**Key I2C formulas (Standard Mode, 100kHz, APB1=36MHz):**
```
FREQ  = 36                                  (MHz)
CCR   = 36,000,000 / (2 × 100,000) = 180
TRISE = 36 + 1 = 37
```

---

## 7. AFIO and EXTI (Button)

| Register | Base Address | Bits Used | Role in the Project |
|---|---|---|---|
| `AFIO_EXTICR1` | `0x40010008` | Bits mapping EXTI0 | Routing the EXTI0 line to GPIOA |
| `EXTI_RTSR` | `0x40010408` | `TR0` | Enables rising-edge detection (button release) |
| `EXTI_FTSR` | `0x4001040C` | `TR0` | Enables falling-edge detection (button press) |
| `EXTI_IMR` | `0x40010400` | `MR0` | Unmasks the interrupt on line 0 |
| `EXTI_PR` | `0x40010414` | `PR0` | Pending interrupt flag (cleared by writing 1) |

---

## 8. SysTick (Cortex-M3 core peripheral, not an external device)

| Register | Base Address | Bits Used | Role in the Project |
|---|---|---|---|
| `SysTick_CTRL` | `0xE000E010` | `ENABLE`, `TICKINT`, `CLKSOURCE` | Enables a 1ms periodic interrupt |
| `SysTick_LOAD` | `0xE000E014` | 24 bits | Reload value (`71999` for a 1ms tick at 72MHz) |

---

## Summary: Peripherals and Their Required Clock Enable

| Peripheral | Bus | Required Enable Bit |
|---|---|---|
| GPIOA/B/C, AFIO | APB2 | `RCC_APB2ENR` |
| PWR, BKP | APB1 | `RCC_APB1ENR` |
| I2C1 | APB1 | `RCC_APB1ENR` |
| RTC | Backup Domain | `RCC_BDCR_RTCEN` (separate from APB1ENR) |
