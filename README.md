# Arduino Multi-Alarm Clock

A customizable, low-power digital alarm clock built with Arduino. Supports multiple alarms with weekday scheduling, a minimalist two-button interface, and a 3D-printed enclosure. Designed for reliability, power efficiency, and a clean user experience.

---

## Features

- Set up to **8 independent alarms** with day-of-week schedules
- **16x4 LCD** with large, custom-rendered block digits
- **EEPROM** storage for alarm persistence across power cycles
- **Two-button UI** with support for short/long presses and menu navigation
- **Light sensor** for automatic screen dimming in dark environments
- **Sleep mode** with minute-level wake-up via RTC interrupt
- **AC-powered** with voltage sensing for brownout detection
- **Custom 3D-printed case** and clean protoboard layout

---

## Hardware

| Component              | Description                                |
|------------------------|--------------------------------------------|
| Arduino Uno            | Microcontroller                            |
| 16x4 LCD (I2C)         | Display with custom characters              |
| DS3231 RTC             | Real-time clock with alarm interrupt       |
| Photoresistor          | Light sensor for ambient brightness        |
| Piezo buzzer           | Alarm sound output                         |
| Push buttons (x2)      | Input interface                            |
| EEPROM (internal)      | Persistent alarm storage                   |
| Power adapter          | AC power supply                            |
| Voltage divider        | Supply voltage monitoring                  |
| 3D-printed enclosure   | Custom-designed and printed case           |

---

## Software Details

- Written in **C++ for Arduino**
- Uses `Wire.h` for I2C communication with RTC and LCD
- Alarm logic uses a **weekday bitmask** and time comparison
- UI implemented via a **finite state machine (FSM)**
- Custom characters defined for large-format digit display
- Power management with **sleep mode** using `LowPower.h`
- Alarms and config saved to **EEPROM** with write buffering

---

## Setup and Usage

1. **Upload the Sketch**

   - Open `AlarmClock.ino` in the Arduino IDE
   - Connect your Arduino via USB
   - Select the correct board and port, then upload

2. **Wiring Guide**

   - Connect the RTC (DS3231) to SDA/SCL (A4/A5 on Uno)
   - Connect the LCD via I2C (same bus as RTC)
   - Wire photoresistor with a voltage divider to an analog pin
   - Connect push buttons to digital pins with pull-down or pull-up resistors
   - Optional: add buzzer to digital output pin for alarm sound
   - Optional: wire voltage divider to analog pin for power sensing

3. **Controls**

   - **Short press (Button A):** Cycle through modes
   - **Long press (Button A):** Enter/confirm current selection
   - **Short press (Button B):** Increment values or toggle options
   - **Long press (Button B):** Cancel or return to previous menu

4. **Modes**

   - View current time and upcoming alarms
   - Set current time
   - Add/edit alarms (hour, minute, active days)
   - Enable/disable alarms
   - View battery/power status

5. **Alarm Behavior**

   - Clock checks alarm schedule every minute on wake
   - If current time and weekday match an active alarm, buzzer sounds
   - Alarm can be dismissed with any button press

---

## Demo

![Alarm Clock Demo](demo.gif)

---

## License

This project is licensed under the MIT License. See `LICENSE` for details.

---
