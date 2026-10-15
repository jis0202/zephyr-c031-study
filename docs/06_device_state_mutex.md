# Device State and Mutex Verification

## Objective

UART 기반 Device Controller에 공유 Device State를 추가하고, Main Thread와 Control Thread가 동일한 상태 데이터에 접근할 때 Zephyr Mutex를 이용하여 데이터 일관성을 보호한다.

## Architecture

```text
UART Command
     |
     +------ LED ON / LED OFF
     |              |
     |              v
     |        Message Queue
     |              |
     |              v
     |        Control Thread
     |              |
     |              +----> GPIO
     |              |
     |              v
     |          Mutex Lock
     |              |
     |              v
     |        Device State
     |              ^
     |              |
     +-- STATUS --> Mutex Lock
```

## Device State

```c
struct device_state {
    bool led_on;
    uint32_t last_event_id;
};
```

The device state records the currently applied LED state and the ID of the last processed control event.

## Synchronization

`state` is shared between the Main Thread and Control Thread.

The Control Thread locks `state_mutex` while updating the state after hardware control.

The Main Thread locks the same mutex while creating a snapshot for the `STATUS` command.

The mutex is released before console output so that the critical section remains short.

## Supported Commands

- `LED ON`
- `LED OFF`
- `STATUS`

Invalid commands are rejected before being delivered to the Control Thread.

## Verification

- Firmware build: PASS
- Hardware flash: PASS
- Initial state (`LED OFF / Event 0`): PASS
- LED ON state update: PASS
- LED OFF state update: PASS
- STATUS command: PASS
- Invalid command state protection: PASS
- Mutex protected shared state access: PASS

## Key Finding

Message Queue와 Mutex는 서로 다른 목적으로 사용된다.

Message Queue는 Thread 사이에서 명령 데이터를 전달하며, Mutex는 여러 Thread가 접근하는 공유 Device State의 일관성을 보호한다.

공유 상태를 출력할 때에는 Mutex가 보호하는 구간에서 상태를 snapshot으로 복사한 후 즉시 Mutex를 해제하고, 비교적 느린 console 출력은 critical section 외부에서 수행하도록 구성하였다.

Build issue:
Application-defined `struct device_state` conflicted with Zephyr's internal
`struct device_state` from <zephyr/device.h>.

Resolution:
Renamed the application structure to `struct app_device_state`.
