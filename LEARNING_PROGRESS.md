# STM32 전체 학습 진행 현황

마지막 업데이트: 2026-08-15

## 현재 위치

`05_Timer_Sampling`에서 TIM2 Hardware Timer Interrupt로 CdS ADC의 500ms 측정 시점을 만들었고, UART 출력과 조도 LED 동작까지 실제 보드에서 확인했다.

## 프로젝트별 진행 상태

| 프로젝트 | 핵심 학습 내용 | 구현 | 빌드 | 보드 검증 | 관련 문서 |
|---|---|---|---|---|---|
| `01_GPIO_LED` | GPIO, 4색 LED 상태, Button Debouncing | 완료 | 완료 | 초기 B1·LD2·PB5 확인, 현재 4상태 전체 추가 확인 필요 | `blog/technologies/GPIO.*` |
| `02_UART` | USART2 Polling/Interrupt, Callback, 문자열 명령 | 완료 | 완료 | 송수신·`led red` 기록 있음, 현재 Blocking `printf()` 추가 확인 필요 | `blog/technologies/UART.*` |
| `03_UART_DMA` | RX/TX DMA, ReceiveToIdle, DMA IRQ/Callback, `__WFI()` | 완료 | 완료 | Flash·Verify·DMA 송수신·LED 명령 완료 | `blog/technologies/DMA.*` |
| `04_ADC_CdS` | PA0/ADC1_IN0, 12-bit ADC, CdS, 조도 LED | 완료 | 완료 | Flash·Verify·밝음/어두움 LED·UART 출력 완료 | `blog/technologies/ADC.*` |
| `05_Timer_Sampling` | TIM2 Update Interrupt, Callback Flag, 주기 Sampling | 완료 | 완료 | Flash·Verify·UART 약 500ms 출력·LED 동작 완료 | `blog/technologies/Timer.*` |
| `06_PWM_LED` | TIM1_CH1 PWM, CdS 값 기반 Duty Cycle | 완료 | 완료 | Flash·Verify·밝기 변화·UART 출력 확인 완료 | `blog/technologies/PWM.*` |

## 04 ADC CdS 검증 기록

- 기본: `raw=587~588`, `473mV`
- 어두움: `raw=3657~3881`, `2.947~3.127V`
- 밝음: `raw=252~308`, `203~248mV`
- 밝음은 `BRIGHT`·초록 LED, 어두움은 `DARK`·빨강 LED로 실제 보드에서 확인 완료

## 05 Timer Sampling 구현 기록

- TIM2 Clock: APB1 Timer Clock 84MHz
- Prescaler `8399` → 10kHz Counter Clock
- Period `4999` → 500ms Update Interrupt
- Callback은 `timerSampleRequested` Flag만 설정
- Main Loop가 ADC Polling·LED 표시·UART 출력을 실행
- Debug 빌드: FLASH 24,484 B / 512 KB, RAM 2,624 B / 96 KB

## 학습 로드맵

| 순서 | 프로젝트 또는 주제 | 핵심 학습 내용 | 현재 상태 |
|---|---|---|---|
| 01 | `01_GPIO_LED` | GPIO 출력, Button Debouncing | 완료 |
| 02 | `02_UART` | USART2 Polling/Interrupt, 문자열 명령 | 완료 |
| 03 | `03_UART_DMA` | RX/TX DMA, ReceiveToIdle, Callback | 완료 |
| 04 | `04_ADC_CdS` | ADC1, CdS 조도 측정, LED 표시 | 완료 |
| 05 | `05_Timer_Sampling` | TIM2 Update Interrupt, 주기 Sampling | 완료 |
| 06 | `06_PWM_LED` | Timer PWM으로 LED 밝기 제어 | 구현·빌드 완료 · 보드 검증 완료 |
| 07 | `07_I2C` | I2C1 주소 스캔, MPU6050 통신 준비 | 구현·Debug 빌드·Flash/Verify 완료 · 주소 응답 추가 확인 필요 |
| 08 | SPI | SPI Peripheral 통신 | 예정 |

`Interrupt`는 별도 프로젝트로 분리하지 않는다. `02_UART`의 UART 수신 Interrupt와 `05_Timer_Sampling`의 TIM2 Update Interrupt에서 IRQ Handler, HAL Callback, Main Loop의 역할 분리를 학습했다.

## 다음 학습 시작점

`06_PWM_LED`은 구현·Debug 빌드·NUCLEO-F401RE Flash·Verify와 LED 밝기 변화·UART 출력을 모두 확인했다. `07_I2C`은 I2C1 주소 스캔 구현·Debug 빌드·Flash/Verify를 완료했으며, 다음 시작점은 I2C 장치 주소 응답 확인이다.

## 상태 기준

- `구현 완료`: Source Code에 기능이 포함됨
- `빌드 완료`: ELF 생성까지 확인함
- `보드 검증 완료`: 실제 NUCLEO-F401RE에서 동작을 확인함
- `추가 확인 필요`: 구현 또는 과거 기록은 있으나 현재 보드 확인이 없음
