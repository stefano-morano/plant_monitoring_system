# Plant Monitoring System

## Overview
The Plant Monitoring System is designed to continuously monitor environmental factors that affect plant health, including soil moisture, light intensity, and GPS data. By using a range of sensors, this system collects real-time data, processes it, and provides feedback through various modes such as Test Mode, Normal Mode, and Advanced Mode. The data is logged and displayed for the user via a serial connection and an RGB LED to indicate different sensor status.

This project leverages embedded platforms, sensors, and communication protocols for Internet of Things (IoT) applications in plant care and environmental monitoring.

## Features
### GPS Data
Reads and parses GPS data (Latitude, Longitude, Altitude, etc.).
### Soil Moisture Sensor
Measures soil moisture and converts the value into a percentage.
### Light Sensor
Measures ambient light intensity and converts the value into a percentage.
### Advanced Diagnostics
Includes alarms and visual indicators for faulty sensors and parameter deviations.
### Multiple Modes
Switch between Test, Normal, and Advanced modes, each with different functionalities and reporting intervals.
### Real-Time Serial Data Output
Outputs sensor data and system status to a serial monitor for user monitoring and debugging.
### RGB LED Indicators
Visual feedback of sensor health and system status using an RGB LED.

## Requirements
### Hardware:

Microcontroller (e.g., STM32, Arduino, etc.)
GPS Module (e.g., NEO-6M)
Soil Moisture Sensor
Light Sensor (e.g., TCS3472)
RGB LED
Push button (for mode switching)

### Software:

Arduino IDE (or other compatible IDE)
Serial Terminal (for monitoring output)

## System Modes
### Test Mode
Function: Reads and outputs the data from all sensors every 2 seconds. If any sensor is malfunctioning, it will indicate which sensor is unavailable.
Indicator: LED 1 on the board is lit.
### Normal Mode
Function: Outputs sensor data every 30 seconds and logs a detailed report (min, max, mean) every hour.
Indicator: LED 2 on the board is lit.
### Advanced Mode
Function: Continuously monitors sensor data and checks for parameter violations or errors. The system highlights issues by changing colors in an RGB LED and logs any detected problems.
Indicator: LED 3 on the board is lit.
## How to Use
### Upload the Code
Load the provided code onto your microcontroller using the appropriate IDE (e.g., Arduino IDE).
Connect the Sensors: Ensure the GPS, Soil Moisture Sensor, and Light Sensor are properly connected to the board.
### Serial Monitor
Open the serial monitor to view real-time sensor data.
### Mode Switching
Press the Blue Button (PB2) to cycle through the available modes (Test Mode, Normal Mode, Advanced Mode).
### Monitor the RGB LED
The RGB LED will indicate system status, with different colors representing different modes and alarm states.
Class Structure
The project is structured with several classes, each responsible for reading and processing data from different sensors. The two main threads run these classes, ensuring that each sensor's data is handled independently for efficient operation.

## Key Classes:
### GPS Class
Handles reading and parsing GPS data.
### Soil Moisture Sensor Class
Reads and processes soil moisture levels.
### Light Sensor Class
Reads and processes light intensity.
### Main Class
Controls the serial output and manages the communication between the classes.
## Troubleshooting
### No GPS Data
Ensure the GPS module is properly connected and has a clear line of sight to the sky for satellite signals.
### Sensor Not Working
Check the wiring and sensor connections. If any sensor is not functioning, the system will print an error message indicating which sensor is unavailable.
Incorrect Output Values: Verify that all sensors are correctly calibrated and receiving power.

## License
This project is open-source. The authors are Stefano Morano and Ana Daria Corpaci. This project is for the Embedded PLatforms for IoT course in Universidad Politecnica de Madrid.
