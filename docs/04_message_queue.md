# Message Queue Verification

## Objective

Zephyr Message Queue를 이용하여 Main Thread와 Control Thread 사이에서 제어 명령 데이터를 전달하는 Producer/Consumer 구조를 구현하고 검증한다.

## Architecture

```text
Main Thread
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

## Message Format

```c
struct control_msg {
    uint32_t event_id;
    bool led_on;
};
```

Main Thread는 1초마다 LED 제어 메시지를 생성하고 Message Queue에 전달한다.

Control Thread는 Queue에서 메시지를 수신한 후 전달받은 `led_on` 값에 따라 GPIO를 제어한다.

## Zephyr APIs

- `K_MSGQ_DEFINE`
- `k_msgq_put`
- `k_msgq_get`
- `K_THREAD_DEFINE`
- `k_msleep`

## Verification

- Firmware build: PASS
- Hardware flash: PASS
- Message transmission: PASS
- Control Thread wake-up: PASS
- FIFO message processing: PASS
- GPIO control: PASS
- UART logging: PASS

## Queue Overflow Test

Producer 주기를 Consumer 처리 주기보다 빠르게 설정하여 Queue에 메시지가 누적되는 상황을 확인하였다.

Queue 최대 메시지 수를 초과했을 때 `k_msgq_put()` 실패를 통해 Queue Full 상태를 확인하였다.

## Key Finding

Semaphore는 이벤트 발생 여부만 전달하지만 Message Queue는 이벤트와 함께 실제 데이터를 Thread 사이에 전달할 수 있다.

이를 통해 통신 처리와 실제 Hardware Control을 서로 다른 Thread로 분리할 수 있으며, 이후 UART Command Parser와 Control Thread 사이의 명령 전달 구조로 확장할 수 있다.
