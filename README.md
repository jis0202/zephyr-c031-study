# STM32C031 Zephyr RTOS Development

## Overview

NUCLEO-C031C6 기반 Zephyr RTOS 개발 및 Git 협업 학습 프로젝트.

## Development Environment

- MCU: STM32C031C6
- Board: NUCLEO-C031C6
- RTOS: Zephyr 4.4.0
- SDK: Zephyr SDK 1.0.1
- IDE: VS Code
- Build: west / CMake / Ninja / GCC
- Flash: STM32CubeProgrammer / ST-LINK

## Progress

- [x] Development environment setup
- [x] Blinky firmware build
- [x] Hardware flashing and LED verification
- [x] GPIO timing modification
- [x] UART console
- [x] RTOS threads and synchronization
<<<<<<< HEAD
- [x] Message queue
=======
- [ ] Message queue
>>>>>>> main
- [ ] Device controller application

## Source

Initial application derived from the Zephyr Blinky sample (Apache-2.0).

## Verification

See `docs/01_bringup.md`.
