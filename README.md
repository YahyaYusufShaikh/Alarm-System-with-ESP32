# Alarm System for College with ESP32

This project implements an alarm system for a college using the ESP32 microcontroller. The system is designed to sound alarms according to a predefined timetable, helping to manage class schedules efficiently.

## Features
- **Time-Based Alarms**: The alarm system is programmed to trigger alarms based on the college's timetable.
- **NTP Server Integration**: The ESP32 connects to a Network Time Protocol (NTP) server to fetch real-time data, ensuring that the system operates with accurate timing.
- **Web Server Interface**: A web-based interface is provided to easily update and manage the timetable. This allows administrators to adjust the schedule remotely as needed.

## Components Used
- **ESP32**: The core microcontroller used for this project.
- **NTP Server**: Provides real-time data to ensure alarms are triggered at the correct times.
- **Web Server**: Used to host the interface where the timetable can be updated.
- **DC Buzzer**: just for a demo purpose
