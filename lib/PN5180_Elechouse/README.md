# PN5180 NFC Library for ESP32 & Arduino

[![License: LGPL v2.1](https://img.shields.io/badge/License-LGPL_v2.1-blue.svg)](LICENSE)
[![Arduino](https://img.shields.io/badge/platform-Arduino%20|%20ESP32-green.svg)](https://www.arduino.cc/)

This is an enhanced driver library for the **PN5180 NFC Module** from [Elechouse](https://www.elechouse.com/product/pn5180-nfc-module/). It is based on the community PN5180 library but tailored to support the advanced hardware features of the industrial edition, such as **Hard Power-Cycle (Hard Reboot)** and **Low Power Card Detection (LPCD)**.

## 📖 Product Overview

The **Elechouse PN5180 Edition** is a high-performance NFC frontend designed for harsh environments and long-range scanning. Unlike generic consumer modules, it features a robust power architecture and hardware fail-safe mechanisms.

### Key Hardware Features
*   **🔌 Industrial Power Stability:** Onboard **80uF Capacitor** & **ME6211 LDO** absorb RF current surges up to 250mA, preventing voltage dips during high-power ISO15693 scans.
*   **🛡️ Fail-Safe "Hard Reboot":** Dedicated **PD/CE Pin** allows the MCU to physically cut power to the PN5180 logic core, ensuring 100% recovery from state-machine lockups.
*   **🌡️ Thermal Stability:** Optimized antenna matching eliminates "hot-dead" drift.
*   **⚡ Flexible Logic Voltage:** Supports **1.8V to 3.6V** logic levels directly (configurable via jumper).

**🛒 Where to Buy:** [Elechouse PN5180 Industrial NFC Module](https://www.elechouse.com/product/pn5180-nfc-module/) 
---


## 🔌 Wiring Example (ESP32)

| PN5180 Module | ESP32 (VSPI) |
| :--- | :--- |
| **5V** | 5V / VIN |
| **GND** | GND |
| **PVDD** | 3V3 |
| **NSS** | GPIO 5 |
| **MOSI** | GPIO 23 |
| **MISO** | GPIO 19 |
| **SCK** | GPIO 18 |
| **BUSY** | GPIO 16 |
| **RST** | GPIO 17 |
| **PD/CE** | **GPIO 22** (Recommended) |

---


This library is compatible with the Arduino IDE.

📦 Supported Protocols
• ISO/IEC 15693 (ICODE SLIX, SLIX 2) - Best in class range
• ISO/IEC 14443 Type A (Mifare)
