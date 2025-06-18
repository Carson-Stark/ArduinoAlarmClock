# Arduino Multi-Alarm Clock

A customizable, low-power digital alarm clock built using an **Arduino Nano**. Supports multiple alarms with per-day scheduling, a minimalist two-button interface, and a compact 3D-printed enclosure. Designed for reliability, power efficiency, and an intuitive user experience.

---

## Features

- Set up to **8 independent alarms** with weekday scheduling
- **16x4 I2C LCD** with large, block-style custom digits
- **EEPROM**-backed persistent alarm storage
- **Two-button interface** with context-aware short/long presses
- **Ambient light detection** to auto-dim or disable display
- **Deep sleep mode** with RTC interrupt wake every minute
- **AC-powered** with voltage sensing for brownout warnings
- **Compact 3D-printed case** for clean and durable packaging

---

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

---

## Software Overview

- Developed in **Arduino C++** using the Arduino IDE
- LCD and RTC communicate via I2C using `Wire.h`
- Alarm system uses a **weekday bitmask** for flexible scheduling
- Finite State Machine (FSM) handles display and user interaction
- Custom glyphs used to render large-format digits across LCD
- `LowPower.h` enables deep sleep to conserve energy
- EEPROM operations buffered to reduce write wear

---

## Setup & Usage

### 1. Upload Firmware

- Open `AlarmClock.ino` in Arduino IDE
- Select **Arduino Nano** as board (ATmega328P)
- Choose correct **COM port**
- Upload sketch to device

### 2. Wiring Guide

| Signal       | Connection         |
|--------------|--------------------|
| SDA/SCL      | A4 / A5 (I2C bus)  |
| Light sensor | A0 (via voltage divider) |
| Buttons      | D2, D3 (with pull-up/down) |
| Buzzer       | D9 (or any PWM pin) |
| Voltage sense| A1 (via resistor divider) |

### 3. Interface Controls

- **Short Press (Button A):** Navigate menu or toggle fields
- **Long Press (Button A):** Confirm or enter sub-modes
- **Short Press (Button B):** Increment or select values
- **Long Press (Button B):** Cancel/exit or return

### 4. Available Modes

- Time display (default)
- Set time
- Add/edit alarm: set time and weekdays
- Enable/disable alarms
- View voltage / brightness status

### 5. Alarm Logic

- Wakes every minute via RTC interrupt
- If current time and day match a set alarm, buzzer triggers
- Alarms can be dismissed by any button press
- Icons indicate upcoming alarm status (solid = soon, hollow = later)

---

## Demo

![Alarm Clock Demo](demo.gif)

---

## License

MIT License. See `LICENSE` for details.