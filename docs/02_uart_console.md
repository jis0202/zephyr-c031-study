# UART Console Verification

## Objective

NUCLEO-C031C6의 ST-LINK Virtual COM Port를 이용하여 Zephyr console 출력을 확인한다.

## Hardware

- Board: NUCLEO-C031C6
- Interface: USART2 via ST-LINK Virtual COM Port
- Baud rate: 115200
- Data bits: 8
- Parity: None
- Stop bits: 1
- Flow control: None

## Test

Zephyr Blinky application을 실행하고 `printf()`를 통해 LED 상태를 UART console로 출력하였다.

## Output

```text
*** Booting Zephyr OS build v4.4.0 ***
LED state: OFF
LED state: ON
LED state: OFF
LED state: ON
```

## Verification

- ST-LINK Virtual COM Port detection: PASS
- Serial connection at 115200 baud: PASS
- Zephyr boot message output: PASS
- Application `printf()` output: PASS

## Key Finding

Zephyr application의 console 출력을 ST-LINK Virtual COM Port를 통해 PC에서 확인할 수 있음을 검증하였다.

이 UART console은 이후 RTOS Thread 동작, Message Queue, command parser 및 디버깅 로그를 확인하는 기본 인터페이스로 사용한다.
