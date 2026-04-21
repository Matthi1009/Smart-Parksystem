<p align="center">
  <img src="images/htl-leoben-logo.png" alt="HTL Leoben" width="480"/>
</p>

---

# 🚗 IoT Parking System

> **Lab Protocol** · Subject: Lab · Class: 2IT · HTL Leoben
>
> **Author:** Alexander Wetzlhütter · **Date:** 13.04.2026

---

## 📋 Table of Contents

- [Idea](#-idea)
- [Components](#-components)
- [System Flowchart](#-system-flowchart)
- [Tinkercad Simulation](#-tinkercad-simulation)
- [Circuit Schematic](#-circuit-schematic)
- [Prototype](#-prototype)
- [Known Issues & Notes](#-known-issues--notes)
- [Code](#-code)

---

## 💡 Idea

The goal of this project is a smart IoT parking system that works as follows:

1. A driver checks in at the entrance using a **keycard (RFID)** at a sensor.
2. Upon successful authentication, a **barrier (servo motor)** opens automatically.
3. An **ultrasonic sensor** detects when a vehicle exits.
4. The system **counts** cars entering and leaving in real time.
5. The **current number of parked vehicles** is displayed on an LCD screen.

---

## 🔧 Components

### Sensors
| Component | Purpose |
|-----------|---------|
| RFID Module (RC522) | Entry authentication via keycard |
| Ultrasonic Sensor (HC-SR04) | Exit detection |

### Actuators
| Component | Purpose |
|-----------|---------|
| LCD Display (16×2) | Shows current car count |
| Servo Motor + Stepper Driver | Controls the barrier gate |

### Microcontroller
| Component | Notes |
|-----------|-------|
| ESP-32 | Main controller; note: Tinkercad simulation uses Arduino UNO as substitute |

---

## 🔄 System Flowchart

The flowchart below illustrates the full logic of the parking system, from entry through barrier control to exit detection and display update.

<p align="center">
  <img src="images/flowchart.png" alt="System Flowchart" width="600"/>
</p>

**Flow summary:**
- **Entry:** RFID sensor reads keycard → sends signal → motor opens barrier
- **Parking:** Ultrasonic sensor monitors occupied spaces (currently display-only)
- **Exit:** Sensor detects departing vehicle → counter updates → LCD refreshes
- **Display:** Screen always shows current count of parked cars

---

## 🖥️ Tinkercad Simulation

> ⚠️ **Note:** The simulation uses an **Arduino UNO** as a stand-in for the ESP-32 since Tinkercad does not natively support the ESP-32. The wiring logic is equivalent.

<p align="center">
  <img src="images/tinkercad-breadboard.png" alt="Tinkercad Breadboard Layout" width="680"/>
</p>

The breadboard layout shows the wiring of the Arduino UNO, the LCD display with contrast potentiometer, the HC-SR04 ultrasonic sensor, and indicator LEDs (red/green).

---

## 📐 Circuit Schematic

The full circuit schematic, generated in Tinkercad, showing all component connections including resistors, LEDs, LCD interface, and the ultrasonic sensor.

<p align="center">
  <img src="images/tinkercad-schematic.png" alt="Circuit Schematic" width="740"/>
</p>

**Key connections:**
- **RFID RC522** → SPI bus on ESP-32
- **HC-SR04** → TRIG / ECHO digital pins
- **LCD 16×2** → parallel data bus (DB0–DB7) + control pins (EN, RW, RS)
- **Servo motor** → PWM-capable pin
- **LEDs (red/green)** → digital output pins via 120Ω resistors

---

## 🔩 Prototype

The physical prototype was built on a breadboard in the lab. It includes the RFID-RC522 module, ultrasonic sensor, servo motor (acting as the barrier gate), and status LEDs.

<p align="center">
  <img src="images/prototype.jpeg" alt="Physical Prototype" width="620"/>
</p>

---

## ⚠️ Known Issues & Notes

| Issue | Description |
|-------|-------------|
| **Breadboard connectivity** | A major problem was that certain rows on the breadboard were not electrically connected — always verify continuity before testing. |
| **ESP-32 console settings** | Serial monitor settings on the ESP-32 were causing garbled output. This has since been corrected and should now be permanently fixed. |
| **Ultrasonic sensor calibration** | Incorrect trigger timing can cause false readings — verify the `delayMicroseconds()` values in code. |
| **Arduino IDE libraries** | Ensure all required libraries (e.g., `MFRC522`, `LiquidCrystal`, `ESP32Servo`) are installed in Arduino IDE before uploading. |

---

## 💻 Code

The source code is available on **GitHub**. Make sure to install all required libraries in the Arduino IDE before flashing the firmware to the ESP-32.

```
📦 Required Libraries
├── MFRC522          (RFID module)
├── LiquidCrystal    (LCD display)
├── ESP32Servo       (servo motor control)
└── NewPing          (ultrasonic sensor — optional)
```

> 🔗 See the GitHub repository linked in the project for the full source code.

---

<p align="center">
  <sub>HTL Leoben · Rohstoffe · Metalle · Logistik · IT · Lab Report 2026</sub>
</p>
