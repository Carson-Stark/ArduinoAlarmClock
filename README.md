# Arduino Multi-Alarm Clock

<img width="1920" height="1080" alt="hero" src="https://github.com/user-attachments/assets/80f35310-b724-4082-b8e0-6428ea75cd26" />

## Project Overview

A customizable, low-power digital alarm clock built using an **Arduino Nano**. Supports multiple alarms with per-day scheduling, a minimalist two-button interface, and a compact 3D-printed enclosure. Designed for reliability, power efficiency, and an intuitive user experience.

### Features

- Set up to **8 independent alarms** with weekday scheduling
- **16x4 I2C LCD** with large, block-style custom digits
- **Two-button interface** with context-aware short/long presses
- **Ambient light detection** to auto-dim or disable display
- **Deep sleep mode** with RTC interrupt wake every minute
- **AC-powered** with voltage sensing for brownout warnings
- **Compact 3D-printed case** for clean and durable packaging

### Project Timeline

- **Date Started:** September 2018
- **Date Completed:** October 2018

## Demo

https://github.com/user-attachments/assets/caf4f362-8fb0-4f28-ab67-8239f7dfc2e4

## Hardware

| Component              | Description                                |
|------------------------|--------------------------------------------|
| Arduino Nano           | Microcontroller (ATmega328P)               |
| 16x4 LCD (I2C)         | Display with custom character support       |
| DS3231 RTC             | Real-time clock with 1-minute interrupt    |
| Photoresistor          | Ambient light sensing                      |
| Piezo buzzer           | Audible alarm output                       |
| Push buttons (x2)      | User input interface                       |
| EEPROM (internal)      | Non-volatile alarm data                    |
| AC adapter (5V)        | Power supply                               |
| Voltage divider        | Power voltage monitoring                   |
| 3D-printed enclosure   | Custom case for electronics                |

### Circuit Diagram

![CircuitDiagram](https://github.com/user-attachments/assets/23b6b788-bdd8-402b-9778-e067e7c106b0)

Public Circuit Designer link: [Alarm Clock Circuit](https://app.cirkitdesigner.com/project/4ae56a12-4e2b-4acc-8b0b-463c14d5e23f)

### Arduino Pins

| Function                     | Pin | Setting (pinMode) |
|------------------------------|-----|-------------------|
| RTC minute‑tick interrupt    | 2   | `INPUT_PULLUP`    |
| External wake interrupt      | 3   | `INPUT_PULLUP`    |
| Buzzer output                | 5   | `OUTPUT`          |
| UI Button A                  | 11  | `INPUT_PULLUP`    |
| UI Button B                  | 10  | `INPUT_PULLUP`    |
| Photo‑resistor ADC           | A0  | Analog input      |

## Software Overview

- Developed in **Arduino C++** using the Arduino IDE
- LCD and RTC communicate via I2C using `Wire.h`
- Alarm system uses a **weekday bitmask** for flexible scheduling
- Finite State Machine (FSM) handles display and user interaction
- Custom glyphs used to render large-format digits across LCD
- `LowPower.h` enables deep sleep to conserve energy

## Setup & Usage

### 1 · Upload Firmware

1. Open **`AlarmClock.ino`** in the Arduino IDE.  
2. Select **Arduino Nano (ATmega328P, 5 V)** under *Tools → Board*.  
3. Choose the correct **COM port**.  
4. Click **Upload**.

### 2 · Setup Wiring

- Ensure you have connected the required components to the Arduino correctly
- Folow the guide above
- Supply the Circuit with 4.5V DC, recommended to get AC -> DC stepdown converter with barrel plug

### 3 · Interface Controls

| Gesture                        | Result                                |
|--------------------------------|---------------------------------------|
| **Short press A**              | Navigate / toggle field               |
| **Long press A**               | Confirm / enter sub‑mode              |
| **Short press B**              | Increment / select value              |
| **Long press B**               | Cancel / back / exit                  |
| **During alarm – tap**         | Snooze 5 min                          |
| **During alarm – hold B ≈2 s** | Stop alarm until next schedule        |
| **Hold B 3s**                  | Disable/Enable alarm function         |

### 4 · Clock Modes

- **Idle:** Large time display + next‑alarm icon
- **Set Time:** Adjust hour/minute
- **Edit Alarm:** Choose index, set time, select weekdays

### 5 · Alarm Flow

1. DS3231 INT triggers every minute → MCU wakes.  
2. Current weekday + time compared to enabled alarm list.  
3. **Match:** Buzzer sounds continuously.  
4. User tap ➜ snooze 5 min; user hold B ➜ stop.  
5. MCU returns to deep sleep until next INT.

## License

MIT License. See `LICENSE` for details.
