# 2048 Game on RGB LED Matrix

This repository contains a high-performance implementation of the game "2048" developed for the Arduino Mega 2560. The system interfaces with a 64x32 RGB LED matrix (HUB75) using low-level hardware control to ensure smooth, flicker-free gameplay.

## Technical Highlights
* **Bare-Metal Performance**: Built using Direct Port Manipulation to bypass slow standard libraries, allowing for high-speed data transmission to the LED matrix.
* **4-bit Color Depth**: Implements Binary Code Modulation (BCM) to achieve 16 levels of brightness per color channel.
* **Asynchronous Architecture**: Game logic is decoupled from the display refresh process using Timer1 and hardware interrupts, ensuring constant frame rates regardless of processing load.
* **Memory Optimization**: Utilizes a pre-calculated buffer strategy to save CPU cycles and maintain system responsiveness.

## Project Structure
The source code is organized within the `src` folder.
* **Latest Version**: `src/FINAL2048.ino` 

## Hardware Requirements
* Arduino Mega 2560
* 64x32 RGB LED Matrix (HUB75 interface)
* External 5V 4A Power Supply
* 4x Tactile Push-buttons (for movements)

## Authors
* **Demian Piodi**: Hardware architecture, low-level HUB75 driver, BCM implementation, and buffer optimization.
* **Tamer Erata**: High-level game logic, tile merging algorithms, and game state management.

---
*Developed for the Computer Architecture (HS25) course at the University of Basel.*
