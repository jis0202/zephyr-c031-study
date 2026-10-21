# Timer and Workqueue Verification

## Objective

Zephyr Kernel Timer와 System Workqueue를 이용하여 UART Device Controller에 주기적인 비동기 상태 출력 기능을 추가하고 실제 NUCLEO-C031C6 하드웨어에서 검증한다.

Timer expiry context와 일반 Thread context의 역할을 분리하여 Timer callback에서는 실제 처리를 수행하지 않고 Work Item을 System Workqueue에 제출하도록 구성한다.

## Architecture

```text
Kernel Timer
     |
     | Periodic Expiry
     v
Timer Callback
     |
     | k_work_submit()
     v
System Workqueue
     |
     v
Status Work Handler
     |
     v
Device State Snapshot
     |
     v
UART Auto Status
```

기존 Device Controller 구조와 함께 동작한다.

```text
UART Command
     |
     v
Main Thread
     |
     v
Message Queue
     |
     v
Control Thread
     |
     +----> GPIO
     |
     v
Device State
     ^
     |
     +---- Timer / Workqueue Status
```

## Timer Configuration

Periodic status interval:

```c
#define AUTO_STATUS_PERIOD_SEC 5
```

Timer는 다음과 같이 시작한다.

```c
k_timer_start(&status_timer,
              K_SECONDS(AUTO_STATUS_PERIOD_SEC),
              K_SECONDS(AUTO_STATUS_PERIOD_SEC));
```

첫 expiration은 5초 후 발생하며 이후 동일한 주기로 반복된다.

## Timer Callback

Timer expiry callback에서는 실제 Device State 처리나 console 출력을 수행하지 않는다.

```c
static void status_timer_handler(struct k_timer *timer)
{
    ARG_UNUSED(timer);

    (void)k_work_submit(&status_work);
}
```

Timer callback은 Work Item을 System Workqueue에 제출한 후 즉시 종료한다.

## Work Handler

실제 상태 확인과 console 출력은 System Workqueue의 Thread context에서 수행한다.

```c
static void status_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);

    print_device_status("auto status");
}
```

`print_device_status()`는 Mutex로 보호된 Device State를 snapshot으로 복사한 뒤 Mutex를 해제하고 UART 출력을 수행한다.

## Context Separation

### Timer Callback

- Timer expiration 처리
- Interrupt context
- Blocking operation을 수행하지 않음
- Work Item 제출만 수행

### Work Handler

- System Workqueue Thread에서 실행
- Device State snapshot 처리
- Mutex 사용
- UART status 출력

이를 통해 시간에 민감한 Timer callback과 상대적으로 긴 Application 작업을 분리하였다.

## Verification

- Firmware build: PASS
- Hardware flash: PASS
- 5-second periodic timer: PASS
- System Workqueue execution: PASS
- Automatic STATUS output: PASS
- Manual STATUS command: PASS
- LED ON command: PASS
- LED OFF command: PASS
- Device State synchronization: PASS
- UART command processing during periodic status operation: PASS

## Key Finding

`k_msleep()`은 현재 Thread의 실행을 일정 시간 중단하지만, `k_timer`는 Kernel에 독립적인 시간 이벤트를 등록한다.

Timer expiry callback은 interrupt context에서 실행되므로 장시간 처리나 blocking 작업을 직접 수행하지 않고, `k_work_submit()`을 이용해 System Workqueue로 처리를 위임하였다.

이를 통해 UART command 처리, Control Thread, periodic state reporting을 서로 독립적인 실행 흐름으로 구성할 수 있었다.
