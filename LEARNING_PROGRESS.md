# STM32 전체 학습 진행 현황

마지막 업데이트: 2026-08-15

## 현재 위치

`GPIO 기초 → UART Polling/Interrupt → UART DMA`를 완료하고 `04_ADC_CdS` 학습을 시작했다. ADC1 Channel 0 기반 CdS 측정과 UART 출력은 구현·빌드까지 완료했으며, 실제 센서 보드 검증이 남아 있다.

## 프로젝트별 진행 상태

| 프로젝트 | 핵심 학습 내용 | 구현 | 빌드 | 보드 검증 | 관련 문서 |
|---|---|---|---|---|---|
| `01_GPIO_LED` | GPIO Input/Output, 4색 LED 상태, 50ms Button Debouncing | 완료 | 완료 | 초기 B1·LD2·PB5 확인, 현재 4상태 전체 확인 기록 없음 | `blog/technologies/GPIO.*` |
| `02_UART` | USART2, Polling/Interrupt, Callback, 문자열 Buffer, LED 명령, `_write()` | 완료 | 완료 | 터미널 송수신과 `led red` 기록 있음, 현재 Blocking `printf()` 재확인 필요 | `blog/technologies/UART.*` |
| `03_UART_DMA` | RX/TX DMA, Stream/Channel, `ReceiveToIdle`, DMA IRQ/Callback, `__WFI()` | 완료 | 완료 | Flash·Verify·DMA 송수신·LED 명령 확인 완료 | `blog/technologies/DMA.*`, `blog/fundamentals/HAL.*`, `blog/fundamentals/CMSIS.*` |
| `04_ADC_CdS` | PA0/ADC1_IN0, 12-bit 단일 변환, 500ms Polling Sampling, UART 출력, 조도 LED 단계 | 완료 | 완료 | ADC 출력 버전 Flash·Verify 완료, LED 단계 보드 검증 필요 | `blog/technologies/ADC.*`, `blog/fundamentals/HAL.*` |

## 완료한 핵심 학습

### GPIO

- GPIO Port와 Pin, Input과 Output
- `SET`, `RESET`, Push-Pull 출력
- 여러 GPIO Port의 LED 제어
- `HAL_GPIO_ReadPin()` 기반 Button 입력
- `HAL_GetTick()` 기반 50ms 비차단 Debouncing
- `enum` 상태와 상태별 LED 출력

### UART

- USART2, PA2/PA3, 115200 bps, 8-N-1
- Polling 송수신과 Interrupt 수신의 차이
- `HAL_UART_Receive_IT()`와 수신 재등록
- IRQ Handler → HAL IRQ Handler → Callback 흐름
- 32바이트 문자열 Buffer, Enter, Backspace, Overflow 처리
- `strcmp()` 기반 문자열 LED 명령
- `_write()`와 `printf()` 연결

### DMA

- Peripheral과 Memory 사이의 DMA 데이터 이동
- DMA1 Stream5 Channel 4 RX, Stream6 Channel 4 TX
- Peripheral 주소 고정과 Memory 주소 증가
- UART Handle과 DMA Handle의 `__HAL_LINKDMA()` 연결
- DMA Clock, NVIC와 Stream IRQ
- `HAL_UARTEx_ReceiveToIdle_DMA()` 기반 가변 길이 수신
- DMA 임시 Buffer와 문자열 명령 Buffer 분리
- `HAL_UART_Transmit_DMA()`와 완료 Callback
- `__WFI()`를 이용한 Interrupt 대기
- Interrupt 방식과 DMA 방식 비교

### ADC

- Analog Input과 12-bit ADC 결과 범위 0~4095
- PA0와 `ADC1_IN0` Channel 연결
- Software Trigger 단일 변환
- `HAL_ADC_Start()` → `HAL_ADC_PollForConversion()` → `HAL_ADC_GetValue()` → `HAL_ADC_Stop()` 흐름
- `HAL_GetTick()` 기반 500ms Sensor Sampling
- 3.3V 기준 mV 환산과 USART2 출력

## 검증 상태 요약

| 항목 | 상태 |
|---|---|
| `01_GPIO_LED` 최신 소스 빌드 | 완료 |
| `02_UART` Debug 빌드 | 완료 |
| `03_UART_DMA` Debug 빌드 | 완료 |
| `03_UART_DMA` Flash 및 Verify | 완료 |
| UART DMA 실제 송수신과 LED 명령 | 완료 |
| `04_ADC_CdS` Debug 빌드 및 ELF 생성 | 완료 |
| `04_ADC_CdS` Flash 및 Verify | 완료 |
| CdS 기본 측정값 (`raw=587~588`, `473mV`)과 UART 출력 | 완료 |
| CdS 어두운 상태 측정값 (`raw=3657~3881`, `2.947~3.127V`) | 완료 |
| CdS 밝은 상태 측정값 (`raw=252~308`, `203~248mV`) | 완료 |
| 실측 범위 기반 LED 단계 표시 코드와 Debug 빌드 | 완료 |
| LED 단계 표시 Flash 및 실제 LED 동작 | 추가 확인 필요 |
| `01_GPIO_LED` 현재 4상태 전체 보드 동작 | 추가 확인 필요 |
| `02_UART` 현재 Blocking `_write()`의 터미널 출력 | 추가 확인 필요 |

## 학습 문서

| 문서 | 기록 내용 |
|---|---|
| `blog/README.md`, `index.html` | 전체 블로그 문서 목차와 분류 |
| `blog/fundamentals/C.md`, `C.html` | 현재 프로젝트에서 사용한 C 문법과 구조 |
| `blog/technologies/GPIO.md`, `GPIO.html` | GPIO 출력, 다중 LED, Button Debouncing |
| `blog/technologies/UART.md`, `UART.html` | UART 송수신과 Interrupt 문자열 명령 |
| `blog/technologies/DMA.md`, `DMA.html` | DMA Stream/Channel, RX/TX 전송과 Callback |
| `blog/technologies/ADC.md`, `ADC.html` | CdS Analog Input, 12-bit 변환과 Polling Sampling |
| `blog/fundamentals/HAL.md`, `HAL.html` | 현재 코드에서 사용한 STM32 HAL API와 Macro |
| `blog/fundamentals/CMSIS.md`, `CMSIS.html` | CMSIS-Core와 `__WFI()` |

## 다음 학습 시작점

현재 `04_ADC_CdS`의 ADC 값 출력은 Flash·Verify까지, 실측 범위 기반 LED 단계 표시는 구현·빌드까지 완료했다. 다음 시작점은 새 Firmware를 Flash하고 밝기별 LED와 UART `brightness` 출력을 확인하는 것이다. 검증 후 다음 확장으로 Timer 기반 주기 Sampling을 진행한다.

## 상태 기준

- `구현 완료`: 현재 Source Code에 기능이 포함됨
- `빌드 완료`: ELF 생성까지 확인됨
- `보드 검증 완료`: 실제 NUCLEO-F401RE에서 동작 확인됨
- `추가 확인 필요`: 구현 또는 과거 기록은 있지만 현재 Source 기준 보드 확인이 남음
