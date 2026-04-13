# IoT Parking System

An intelligent parking management system based on the ESP-32 that monitors entries and exits and displays the current capacity in real-time.

## 📋 Project Overview
[cite_start]The system enables automated parking space management[cite: 2]. [cite_start]Users check in via an RFID keycard, which triggers a barrier to open[cite: 3]. [cite_start]Sensors detect vehicle movements to calculate the current number of parked cars, which is then shown on a display[cite: 4].

## 🛠 Hardware Components
[cite_start]Based on the current design, the following components are used[cite: 5]:

* [cite_start]**Microcontroller:** ESP-32 [cite: 5]
* [cite_start]**Sensors:** * RFID modules (for check-in) [cite: 5]
    * [cite_start]Ultrasonic sensors (for vehicle detection) [cite: 5]
* **Actuators:**
    * [cite_start]Display (status & capacity indicator) [cite: 5]
    * [cite_start]Motor including stepper driver (for barrier control) [cite: 5]
* **Miscellaneous:**
    * [cite_start]Rotary potentiometer (required for VO / contrast adjustment) [cite: 1]
    * [cite_start]Voltage converter (specifically required for Echo components) [cite: 1]
    * [cite_start]Resistors (Note: 1k Ohm resistors have slightly thicker rings) [cite: 1]

## 🔍 Important Technical Notes
* [cite_start]**Power Supply:** A dedicated voltage converter is mandatory for "Echo"[cite: 1]. [cite_start]The converter's voltage must be checked before commissioning[cite: 1].
* [cite_start]**AI Support:** An external AI is planned to be used for validating the pin assignment and for a general system check[cite: 1].
* [cite_start]**Simulation:** The initial setup and testing will be performed virtually in **Tinkercad**[cite: 5].

## 🚀 How It Works
1. [cite_start]**Check-in:** Scanning the keycard at the RFID sensor[cite: 3].
2. [cite_start]**Access:** Automatic opening of the barrier[cite: 3].
3. [cite_start]**Detection:** Ultrasonic sensors register entering and exiting vehicles[cite: 4].
4. [cite_start]**Output:** The ESP-32 calculates the difference and displays the number of currently parked cars on the screen[cite: 4].

---
**Project Details:**
* [cite_start]**Subject:** Laboratory [cite: 2]
* [cite_start]**Project Name:** IOT-Parksystem [cite: 2]