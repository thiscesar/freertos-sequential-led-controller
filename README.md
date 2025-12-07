# FreeRTOS Sequential LED Controller

A real-time sequential LED control system using Arduino UNO, FreeRTOS, and TM1638 module.

## Overview

This project implements a 4-LED sequential controller where each LED stays on for a configurable time. Built for an Operating Systems course to demonstrate multitasking concepts.

## Hardware

- Arduino UNO
- TM1638 Module (8 LEDs, 8 buttons, 8-digit display)

## Features

- Configurable timing for each LED (SW1-SW4)
- Start sequence (SW5)
- Pause/Resume (SW6)
- Reset (SW7)
- Decrement modifier (SW8)
- Real-time display update
- Serial output monitoring

## Wiring

| TM1638 | Arduino |
|--------|---------|
| VCC | 5V |
| GND | GND |
| STB | Pin 11 |
| CLK | Pin 12 |
| DIO | Pin 13 |

## Dependencies

- [Arduino_FreeRTOS](https://github.com/feilipu/Arduino_FreeRTOS_Library)
- [TM1638 by dvarrel](https://github.com/dvarrel/TM1638)
