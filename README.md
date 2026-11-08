# ESP32-Jam

A high-performance 2.4 GHz signal jammer built on the **ESP32-WROOM** platform. It utilizes two independent **nRF24L01+PA+LNA** modules to sweep the 2.4 GHz spectrum in parallel, flooding the band with noise at maximum speed and power.

---

## ⚠️ LEGAL DISCLAIMER & WARNING

**INTENTIONAL RADIO FREQUENCY INTERFERENCE (JAMMING) IS STRICTLY ILLEGAL IN MOST COUNTRIES.**

*   Operating this device in public spaces, or using it to disrupt communications (Wi-Fi, Bluetooth, drones, etc.) that you do not own, is a severe violation of telecommunications laws.
*   Penalties can include heavy fines, equipment seizure, and imprisonment.
*   **This project is provided strictly for educational purposes, hardware prototyping, and RF research.** 
*   You must only test this device in a controlled environment (such as a Faraday cage) and on equipment you personally own. The author assumes no responsibility for any misuse, damage, or legal consequences caused by this software or hardware.

---

## 🚀 Features

*   **Dual-Band Parallel Sweeping:** Module 1 sweeps channels 0-62, while Module 2 sweeps channels 63-125 simultaneously, effectively covering the entire 2.4 GHz band in half the time.
*   **Aggressive TX Optimization:** Bypasses standard protocol overhead by disabling Auto-ACK, disabling auto-retransmission.
*   **Power Path Circuit:** Dedicated hardware design for seamless switching between USB power and LiPo battery, preventing battery over-discharge.

---

## 🛠️ Hardware Requirements

*   1x ESP32-WROOM-32 (38-pin)
*   2x nRF24L01+ PA+LNA Modules (with external antennas)
*   1x TP4056 Li-Ion/LiPo Charger Module (with DW01A protection)
*   1x Single-cell 3.7V LiPo Battery
*   1x P-Channel Logic-Level MOSFET (SI2301A-TP)
*   1x Schottky Diode (1N5819)
*   1x Low-Dropout (LDO) Regulator (MIC29150-3.3WT).
*   1x SPDT Slide Switch (Power ON/OFF)
*   1x Push Button.
*   2x LEDs (Red for Power, Green for Status)
---

## 🔌 Pinout & Wiring

### SPI Bus (Shared by both radios)
| nRF24 Pin | ESP32 Pin | Notes |
| :--- | :--- | :--- |
| **SCK** | GPIO 18 | Shared |
| **MISO** | GPIO 19 | Shared |
| **MOSI** | GPIO 23 | Shared |

### Radio Modules
| Module | Pin | ESP32 Pin | Channels Swept |
| :--- | :--- | :--- | :--- |
| **Radio 1** | CE | GPIO 5 | 0 to 62 |
| **Radio 1** | CSN | GPIO 22 | - |
| **Radio 2** | CE | GPIO 21 | 63 to 125 |
| **Radio 2** | CSN | GPIO 4 | - |

---

## ⚡ Hardware & Power Path Notes

1.  **The "Power Path" Circuit:** The LiPo battery must be connected to `BAT+`/`BAT-` on the TP4056. The system must be powered from `OUT+` (the protected output). The MOSFET (Source) connects to `OUT+`, its Drain connects to the LDO input, and the Schottky diode bridges the USB `IN+` to the LDO input. This ensures the battery charges safely while the USB is plugged in, and takes over instantly when unplugged.
2.  **DO NOT cross the USBs:** The ESP32 is powered directly via its `3V3` pin, bypassing its onboard regulator. **Never** connect a USB cable to the ESP32 dev board for programming while the device is powered by the battery or charging via the TP4056. It will cause a voltage conflict and permanently damage the ESP32 or the LDO. Turn the slide switch OFF before flashing.
3.  **Capacitors:** The 100µF capacitors **must** be soldered as close as physically possible to the VCC and GND pins of the nRF24 modules. Transmitting at maximum power draws sharp current spikes that will cause brownouts and reset the ESP32 if not decoupled properly.