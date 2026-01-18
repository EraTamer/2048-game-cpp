# 2048 Game on RGB LED Matrix

[cite_start]This repository contains a high-performance, standalone implementation of the "2048" puzzle game for the **Arduino Mega 2560**[cite: 3, 33]. [cite_start]The project interfaces directly with a **64x32 RGB LED matrix** using the HUB75 protocol without relying on high-level graphics libraries[cite: 9, 34].

### Key Technical Features
* [cite_start]**Bare-Metal Hardware Control**: Utilizes **Direct Port Manipulation** to achieve high-speed parallel data transmission[cite: 11, 28].
* [cite_start]**Binary Code Modulation (BCM)**: Implements a custom 4-bit color depth (16 levels of brightness) through precise timing control[cite: 11, 29, 46].
* [cite_start]**Asynchronous Architecture**: Decouples display refreshing from game logic using **Timer1** and hardware interrupts, ensuring a flicker-free refresh rate[cite: 12, 31, 68].
* [cite_start]**Optimized Memory Management**: Employs a pre-calculated buffer strategy to minimize CPU overhead and maintain responsiveness during intensive multiplexing[cite: 15, 30, 57].

### Repository Structure
The project source code is organized within the `src` directory.
* **Latest Version**: `src/FINAL2048.ino`

### Authors
* [cite_start]**Demian Piodi**: Hardware architecture, low-level HUB75 driver, BCM implementation, and buffer optimization[cite: 5, 119].
* [cite_start]**Tamer Erata**: High-level game logic, tile merging algorithms, score tracking, and state management[cite: 5, 120].

---
[cite_start]*Developed as part of the Computer Architecture (HS25) course at the University of Basel[cite: 1, 8].*
