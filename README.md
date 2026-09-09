# smart-campus-x-ai-iot
SMART CAMPUS X is an AI-powered IoT Smart Campus and Smart Classroom Monitoring System built using ESP32-S3. It provides real-time monitoring of classroom energy consumption, temperature, humidity, air quality, presence, and fire safety, with local OLED display, smart light control, and a web-based dashboard for intelligent campus management.
# SMART CAMPUS X

## AI-Powered Smart Campus Intelligence, Digital Twin & Decision Support System

### Sense • Understand • Predict • Recommend • Verify • Learn

SMART CAMPUS X is an IoT-based Smart Campus and Smart Classroom Intelligence System designed to improve classroom safety, energy efficiency, environmental monitoring, and intelligent campus management.

The system uses ESP32-S3 as the main controller and integrates multiple sensors, electrical energy monitoring, local OLED visualization, smart lighting control, and a real-time web dashboard.

---

## 🚀 Key Features

- Real-time classroom energy monitoring
- Voltage monitoring
- Current monitoring
- Power monitoring
- Frequency monitoring
- Power factor monitoring
- Energy consumption monitoring
- Temperature monitoring
- Humidity monitoring
- Air-quality monitoring
- Flame/fire detection
- Classroom presence detection
- Ambient light monitoring
- Smart classroom light control
- OLED local display
- Real-time web dashboard
- Classroom-wise monitoring architecture
- Energy analysis
- Safety alerts
- Expandable campus architecture

---

## 🧠 System Concept

SMART CAMPUS X follows a closed-loop intelligence architecture:

SENSE  
↓  
UNDERSTAND  
↓  
ANALYZE  
↓  
EXPLAIN  
↓  
PREDICT  
↓  
RECOMMEND  
↓  
ADMIN DECISION  
↓  
CONTROL  
↓  
VERIFY  
↓  
MEASURE RESULT  
↓  
LEARN  
↓  
IMPROVE

---

## 🔧 Hardware

### Main Controller

- ESP32-S3

### Sensors and Modules

- PZEM-004T
- DHT11
- MQ-135
- Flame Sensor
- HC-SR501 PIR
- LDR
- SSD1306 OLED
- Relay Module

### Load

- 60 W classroom bulb

---

## 📡 GPIO Configuration

| Component | ESP32-S3 Pin |
|---|---:|
| DHT11 DATA | GPIO 20 |
| MQ-135 Analog | GPIO 1 |
| LDR Analog | GPIO 2 |
| Flame Sensor | GPIO 3 |
| PIR OUT | GPIO 6 |
| Relay | GPIO 10 |
| OLED SDA | GPIO 8 |
| OLED SCL | GPIO 9 |
| PZEM RX | GPIO 16 |
| PZEM TX | GPIO 17 |
| Push Button | GPIO 21 |

> Verify the exact ESP32-S3 board pin availability before hardware assembly.

---

## ⚡ Energy Monitoring

The PZEM-004T is used to monitor electrical parameters such as:

- Voltage
- Current
- Power
- Frequency
- Power Factor
- Energy

Classroom 1 is the physically instrumented classroom in the current prototype.

Additional classrooms are represented as an expandable architecture and require their own physical measurement hardware for real measurements.

---

## 🌡️ Environmental Monitoring

DHT11 measures:

- Temperature
- Humidity

MQ-135 provides a prototype air-quality indication.

LDR provides relative ambient-light measurement.

---

## 🔥 Safety Monitoring

The flame sensor detects the presence of flame and can generate a safety warning.

The PIR sensor detects movement/presence within the classroom area.

> PIR is used for presence detection and does not provide an exact human count.

---

## 💡 Smart Light Control

The classroom light is controlled using a relay connected to the ESP32-S3.

When the light is turned ON:

- Relay activates
- Classroom bulb turns ON
- Electrical parameters can be monitored

When the light is turned OFF:

- Relay deactivates
- Classroom bulb turns OFF
- Load power/current should decrease accordingly when the PZEM is measuring that load.

---

## 🌐 Web Dashboard

The dashboard provides a centralized view of:

- Classroom status
- Energy parameters
- Environmental conditions
- Safety status
- Presence status
- Light status
- Historical/live information

The interface is designed for simple and easy monitoring.

---

## 🏫 Classroom Architecture

### Classroom 1

Physical prototype:

- ESP32-S3
- PZEM-004T
- 60 W bulb
- Environmental sensors
- Safety sensors
- Relay control

### Classroom 2

Reserved for future physical expansion.

### Classroom 3

Reserved for future physical expansion.

### Classroom 4

Reserved for future physical expansion.

Multiple classrooms can be integrated by deploying additional IoT nodes and measurement hardware.

---

## 🔄 Working Flow

1. Sensors collect classroom information.
2. ESP32-S3 receives the sensor data.
3. The controller processes the information.
4. Electrical parameters are monitored.
5. Environmental conditions are evaluated.
6. Safety sensors detect abnormal conditions.
7. Data is displayed on the OLED.
8. Data is presented on the web dashboard.
9. Authorized users can control the classroom light.
10. The system observes the resulting classroom state.
11. Energy information can be analyzed for improvement.

---

## 🎯 Problem Statement

Traditional classrooms often lack centralized real-time monitoring of energy consumption, environmental conditions, safety, and classroom activity.

Lights and electrical equipment may remain ON unnecessarily, while environmental or safety problems may go unnoticed.

SMART CAMPUS X addresses these challenges through real-time IoT monitoring and intelligent campus management.

---

## 💡 Solution

SMART CAMPUS X combines sensors, ESP32-S3 processing, electrical energy monitoring, local visualization, smart control, and a web dashboard into a unified smart classroom platform.

The platform can be expanded from a single classroom prototype to multiple classrooms and eventually an entire campus.

---

## 📦 Project Structure

```text
firmware/       → ESP32-S3 firmware
hardware/       → Circuit and hardware information
dashboard/      → Web dashboard
documentation/  → Project documentation
presentation/   → PPT and presentation material
images/         → Project photographs and diagrams
demo/           → Demonstration videos
