# 01. Board Bring-up

## Objective
STM32C031C6에서 Zephyr RTOS 기반 GPIO 제어 검증.

## Procedure
1. Zephyr 4.4.0 및 ARM toolchain 구성
2. NUCLEO-C031C6 대상 Blinky 빌드
3. ST-LINK를 통한 firmware 다운로드
4. 실제 LD4 LED 동작 확인

## Test Results

| Item | Result |
|---|---|
| Build | PASS |
| Flash | PASS |
| GPIO LED blinking | PASS |
| LED toggle interval | 1000 ms (configured) |
| Flash usage | 14,308 B / 32 KB |
| RAM usage | 4,088 B / 12 KB |

## Next Steps
- LED toggle interval 변경 및 재검증
- Devicetree GPIO alias 분석
- UART console 검증
