# STM32 Embedded Side Project

NUCLEO-F401RE로 STM32 주변장치와 HAL 기반 동작을 단계별로 구현하고, 빌드와 실제 보드 검증 결과를 함께 기록하는 학습 저장소입니다.

## 현재 진행 상태

현재 `06_PWM_LED`까지 구현·Debug 빌드·NUCLEO-F401RE 보드 검증을 완료했습니다. TIM1 Channel 1이 PA8(D7)의 빨간 LED를 PWM으로 구동하고, Main Loop가 CdS ADC 측정값으로 Duty Cycle을 갱신합니다.

## 학습 로드맵

`GPIO → UART/Interrupt → UART DMA → ADC(CdS) → Timer Sampling → PWM → I2C → SPI`

`Interrupt`는 별도 프로젝트가 아니라 UART와 Timer 프로젝트에서 함께 학습했습니다. 다음 학습 챕터는 I2C입니다.

| 프로젝트 | 주제 | 구현 | 빌드 | 보드 검증 |
|---|---|---|---|---|
| [01_GPIO_LED](01_GPIO_LED/) | GPIO, LED, Button Debouncing | 완료 | 완료 | 일부 추가 확인 필요 |
| [02_UART](02_UART/) | USART2 Polling/Interrupt, 문자열 명령 | 완료 | 완료 | 일부 추가 확인 필요 |
| [03_UART_DMA](03_UART_DMA/) | UART RX/TX DMA, ReceiveToIdle, Callback | 완료 | 완료 | 완료 |
| [04_ADC_CdS](04_ADC_CdS/) | ADC1 PA0, CdS, 조도별 LED | 완료 | 완료 | 완료 |
| [05_Timer_Sampling](05_Timer_Sampling/) | TIM2 Update Interrupt, 500ms Sampling | 완료 | 완료 | 완료 |
| [06_PWM_LED](06_PWM_LED/) | TIM1_CH1 PWM, CdS 기반 LED 밝기 제어 | 완료 | 완료 | Flash·Verify·밝기 변화·UART 출력 확인 완료 |
| [07_I2C](07_I2C/) | I2C1 주소 스캔, MPU6050 통신 준비 | 구현·빌드 완료 | 완료 | 주소 응답 추가 확인 필요 |

세부 상태와 다음 학습 시작점은 [LEARNING_PROGRESS.md](LEARNING_PROGRESS.md)에서 확인할 수 있습니다.

## 기술 문서

구현한 내용은 프로젝트 폴더와 별도로 기술별 문서로 정리합니다. 전체 목록은 [blog/README.md](blog/README.md), HTML 목록은 [blog/index.html](blog/index.html)에서 볼 수 있습니다.

| 구분 | 문서 |
|---|---|
| 기반 지식 | [C](blog/fundamentals/C.md) · [HAL](blog/fundamentals/HAL.md) · [CMSIS](blog/fundamentals/CMSIS.md) |
| 주변장치 | [GPIO](blog/technologies/GPIO.md) · [UART](blog/technologies/UART.md) · [DMA](blog/technologies/DMA.md) · [ADC](blog/technologies/ADC.md) · [Timer](blog/technologies/Timer.md) |

## Timer Sampling 흐름

```text
TIM2 Update Interrupt (500ms)
  → timerSampleRequested = 1
  → Main Loop
  → ADC1 PA0 측정 / LED 갱신 / UART 출력
```

Interrupt Callback은 Flag만 설정합니다. 시간이 걸릴 수 있는 ADC Polling과 `printf()`는 Main Loop에서 실행합니다.

## 빌드

각 프로젝트 폴더에서 Debug preset을 사용합니다.

```powershell
cmake --preset Debug
cmake --build --preset Debug -j
```

`05_Timer_Sampling`은 NUCLEO-F401RE에서 Flash·Verify, UART 약 500ms 출력, 밝음·어두움 LED 동작까지 확인했습니다.

## 하드웨어

- Board: NUCLEO-F401RE
- ADC 입력: PA0 / ADC1_IN0 (CdS)
- Timer: TIM2 Update Interrupt
- UART: USART2
