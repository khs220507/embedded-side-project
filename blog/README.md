# STM32 임베디드 학습 블로그

프로젝트에서 직접 구현하고 확인한 내용을 공통 기반 지식과 기술 학습으로 나누어 기록한다. HTML 목록은 [index.html](index.html)에서 볼 수 있다.

## 기반 지식

| 문서 | 형식 |
|---|---|
| C 언어 | [Markdown](fundamentals/C.md) · [HTML](fundamentals/C.html) |
| HAL | [Markdown](fundamentals/HAL.md) · [HTML](fundamentals/HAL.html) |
| CMSIS | [Markdown](fundamentals/CMSIS.md) · [HTML](fundamentals/CMSIS.html) |

## 기술 학습

| 문서 | 내용 | 형식 |
|---|---|---|
| GPIO | LED, Button Debouncing | [Markdown](technologies/GPIO.md) · [HTML](technologies/GPIO.html) |
| UART | Polling/Interrupt, 문자열 명령, `printf()` | [Markdown](technologies/UART.md) · [HTML](technologies/UART.html) |
| DMA | UART RX/TX DMA, IRQ, Callback | [Markdown](technologies/DMA.md) · [HTML](technologies/DMA.html) |
| ADC | CdS Analog Input, 12-bit Polling | [Markdown](technologies/ADC.md) · [HTML](technologies/ADC.html) |
| Timer | TIM2 Update Interrupt, 500ms Sampling | [Markdown](technologies/Timer.md) · [HTML](technologies/Timer.html) |
| PWM | TIM1_CH1, CdS 기반 LED 밝기 제어 | [Markdown](technologies/PWM.md) · [HTML](technologies/PWM.html) |

## 면접 대비

| 문서 | 내용 | 형식 |
|---|---|---|
| Embedded SW | 실제 구현·보드 검증 기반 핵심 질문과 답 | [Markdown](interview/Embedded_SW.md) · [HTML](interview/Embedded_SW.html) |

전체 진행 상태와 다음 단계는 루트의 [LEARNING_PROGRESS.md](../LEARNING_PROGRESS.md)에서 관리한다.
