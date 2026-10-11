# UART Command Controller Verification

## Objective

UART Console을 통해 외부 명령을 입력받고, 명령을 해석한 뒤 Message Queue를 통해 Control Thread에 전달하여 GPIO를 제어하는 구조를 구현하고 검증한다.

## Architecture

```text
PC Serial Terminal
        |
        | LED ON / LED OFF
        v
UART Console
        |
        v
Command Parser
        |
        | control_msg
        v
Message Queue
        |
        v
Control Thread
        |
        v
GPIO LED
```

## Supported Commands

- `LED ON`
- `LED OFF`

지원되지 않는 문자열은 Invalid Command로 처리하고 Control Thread에는 전달하지 않는다.

## Processing Flow

1. Main Thread가 UART Console에서 명령을 대기한다.
2. 입력 문자열을 Command Parser에서 해석한다.
3. 정상 명령을 `control_msg` 구조체로 변환한다.
4. Message Queue를 통해 Control Thread에 전달한다.
5. Control Thread가 메시지를 수신하여 GPIO를 제어한다.

## Zephyr APIs

- `console_getline_init`
- `console_getline`
- `K_MSGQ_DEFINE`
- `k_msgq_put`
- `k_msgq_get`
- `K_THREAD_DEFINE`

## Verification

- Firmware build: PASS
- Hardware flash: PASS
- UART command reception: PASS
- `LED ON` command: PASS
- `LED OFF` command: PASS
- Message Queue transfer: PASS
- Control Thread execution: PASS
- Invalid command rejection: PASS

## Key Finding

UART 통신 처리와 Hardware Control을 직접 연결하지 않고 Command Parser와 Message Queue를 경계로 분리하였다.

이를 통해 외부 통신 입력과 실제 Device Control을 서로 다른 실행 영역으로 분리하고, 향후 다양한 명령 및 Device State를 추가할 수 있는 구조를 구성하였다.
