# ESP32-JAM

A high-performance 2.4 GHz signal jammer built on the **ESP32-WROOM** platform. It utilizes two independent **nRF24L01+PA+LNA** modules to sweep the 2.4 GHz spectrum in parallel, flooding the band with noise at maximum speed and power.

---

> [!WARNING]
> **DO NOT cross the USBs:** The ESP32 is powered directly via its `3V3` pin, bypassing its onboard regulator. **Never** connect a USB cable to the ESP32 dev board for programming while the device is powered by the battery or charging via the TP4056. It will cause a voltage conflict and permanently damage the ESP32 or the LDO. Turn the slide switch OFF before flashing.