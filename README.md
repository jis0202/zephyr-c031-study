# STM32C031 Zephyr RTOS Development

## Overview

NUCLEO-C031C6 기반으로 Zephyr RTOS의 기본 구조와 Thread 간 동기화, Message Queue 기반 데이터 전달 및 Git 협업 흐름을 학습하고 실제 하드웨어에서 검증하는 프로젝트입니다.

## Development Environment

- MCU: STM32C031C6
- Board: NUCLEO-C031C6
- RTOS: Zephyr 4.4.0
- SDK: Zephyr SDK 1.0.1
- IDE: VS Code
- Build: west / CMake / Ninja / GCC
- Flash: STM32CubeProgrammer / ST-LINK
- Debug / Console: ST-LINK Virtual COM Port

## Progress

- [x] Development environment setup
- [x] Blinky firmware build
- [x] Hardware flashing and LED verification
- [x] GPIO timing modification
- [x] UART console verification
- [x] RTOS thread and semaphore synchronization
- [x] Message queue based inter-thread communication
- [ ] UART command processing
- [ ] Device controller application

## Implemented Concepts

- Zephyr application build and flash workflow
- Devicetree based GPIO control
- UART console logging
- Multi-thread execution
- Semaphore based thread synchronization
- Message Queue based Producer / Consumer structure
- Feature branch / Pull Request / Merge workflow

## Source

The initial application was derived from the Zephyr Blinky sample distributed under the Apache-2.0 license.

The project is being incrementally extended into a Zephyr RTOS based device controller.

## Verification

Detailed development and verification records are available in:

- `docs/01_bringup.md`
- `docs/02_uart_console.md`
- `docs/03_thread_semaphore.md`
- `docs/04_message_queue.md`

All implemented features are verified on the NUCLEO-C031C6 hardware before being merged into the `main` branch.
