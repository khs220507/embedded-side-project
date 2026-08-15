# Project 04 — ADC CdS Sensor

NUCLEO-F401RE의 PA0(`ADC1_IN0`, Arduino A0)에서 CdS 조도 센서 모듈의 Analog Output을 읽는 학습 프로젝트다. `03_UART_DMA`를 기반으로 만들었으며 기존 UART DMA와 LED 명령 기능을 유지한다.

## 현재 구현

- ADC1 Channel 0, 12-bit, Right Alignment
- Software Trigger 단일 변환
- `ADC_SAMPLETIME_84CYCLES`
- `HAL_GetTick()` 기반 500ms 측정 주기
- ADC 원시값 0~4095와 3.3V 기준 환산 mV 출력
- 실측 범위 기반 4단계 조도 LED 표시
- USART2 DMA 기반 `printf()` 출력

## 배선

| CdS 모듈 | NUCLEO-F401RE |
|---|---|
| `VCC` | `3.3V` |
| `GND` | `GND` |
| `AO` | `A0` / `PA0` |

ADC 입력 전압이 3.3V를 넘지 않도록 한다. `DO` Pin은 현재 사용하지 않는다.

## 예상 출력 형식

```text
CdS raw=252, voltage=203 mV, brightness=BRIGHT
```

| ADC 원시값 | 조도 상태 | LED |
|---:|---|---|
| 0~1200 | `BRIGHT` | 초록 |
| 1201~2100 | `NORMAL` | 파랑 |
| 2101~3000 | `DIM` | 노랑 |
| 3001~4095 | `DARK` | 빨강 |

## 빌드

```powershell
cmake --preset Debug
cmake --build --preset Debug -j
```

Debug 빌드에서 `build/Debug/04_ADC_CdS.elf` 생성을 확인했고, LED 단계 표시 버전도 NUCLEO-F401RE에 Flash·Verify했다. 밝기별 LED 색과 UART `brightness` 출력이 일치하는 것도 실제 보드에서 확인했다.
