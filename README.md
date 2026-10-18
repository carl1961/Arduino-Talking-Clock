
<div align="center">

# 🗣️ Arduino Talking Clock

*A reliable, standalone desktop talking clock with voice synthesis, hourly chimes, a morning alarm, and LED animations.*
Programed by Gemini AI
[Features](#features) • [Hardware Requirements](#hardware-requirements) • [Wiring & Pinout](#wiring--pinout) • [SD Card Setup](#sd-card-setup) • [Installation](#installation)

</div>

---

## 🚀 Features
* **100% Standalone:** Uses a DS3231 Real-Time Clock module. No flaky Wi-Fi connections or internet required!
* **Dynamic Audio Synthesis:** Automatically pieces together hour, tens, and unit MP3 files on the fly.
* **Smart Announcements:** Automatically speaks on 15-minute intervals (00, 15, 30, 45) and on demand via a push button.
* **Morning Rooster Alarm:** Triggers a custom `rooster.mp3` alarm strictly at 6:01 AM using built-in 24-hour time logic.
* **Hardware Time-Setting Menu:** Easily adjust time on the fly using dedicated mode and increment buttons paired with a TM1637 display.
* **Visual Lighting Feedback:** Features addressable NeoPixel LEDs that pulse when the clock speaks.

---

## 🛠️ Hardware Requirements
* **Arduino Uno** (or compatible board)
* **HW-261 VS1053 MP3 Shield** (with MicroSD card)
* **DS3231 RTC Module**
* **TM1637 4-digit 7-segment Display**
* **Addressable LED Ring/Strip** (WS2812B / NeoPixel)
* **PAM8403 Audio Amplifier Module** + 3W Speaker
* **3x Push Buttons** & **JST Connectors**

---

## 📌 Pin Mapping

| Component | Arduino Pin |
| :--- | :--- |
| **VS1053 Reset** | Pin 8 |
| **VS1053 CS** | Pin 6 |
| **VS1053 DCS** | Pin 7 |
| **VS1053 DREQ** | Pin 2 |
| **SD Card CS** | Pin 9 |
| **Say Time Button** | Pin 5 *(to GND)* |
| **Mode Button** | Pin 4 *(to GND)* |
| **Increment Button** | Pin 10 *(to GND)* |
| **TM1637 CLK** | Analog Pin A0 |
| **TM1637 DIO** | Analog Pin A1 |
| **NeoPixel DIN** | Analog Pin A2 |
| **DS3231 RTC** | SDA (`A4`) & SCL (`A5`) |

---

## 📁 MicroSD Card Structure
Format your MicroSD card to **FAT32**. Place all `.mp3` files directly in the **root directory** using lowercase 8.3 naming conventions:

> ⚠️ **Note:** Do not place files inside subfolders; they must be loose in the root directory.

* **Hours:** `h1.mp3` through `h12.mp3`
* **Minutes & Tens:** `m0.mp3` (O'Clock), `m1.mp3` through `m20.mp3`, `m30.mp3`, `m40.mp3`, `m50.mp3`
* **Units (for 21–59):** `u1.mp3` through `u9.mp3`
* **Alarm:** `rooster.mp3`

---

## 📦 Required Libraries
Install the following libraries via the Arduino IDE Library Manager:
* `Adafruit_VS1053`
* `RTClib` (by Adafruit)
* `TM1637Display` (by Avishay Orpaz)
* `Adafruit_NeoPixel`
