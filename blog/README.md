# STM32 임베디드 학습 블로그

프로젝트에서 직접 구현하고 확인한 내용을 기반 지식과 기술 학습으로 나누어 한곳에서 관리한다. 웹 형식으로 보려면 [`index.html`](index.html)을 연다.

## 기반 지식

| 문서 | 내용 | 형식 |
|---|---|---|
| C 언어 | 포인터, 구조체와 레지스터, `enum` 상태 머신, 고정 폭 정수형 | [Markdown](fundamentals/C.md) · [HTML](fundamentals/C.html) |
| HAL | 현재 코드에서 사용한 STM32 HAL 함수, Macro, Handle | [Markdown](fundamentals/HAL.md) · [HTML](fundamentals/HAL.html) |
| CMSIS | CMSIS-Core 계층과 `__WFI()` | [Markdown](fundamentals/CMSIS.md) · [HTML](fundamentals/CMSIS.html) |

## 기술 학습

| 문서 | 내용 | 형식 |
|---|---|---|
| GPIO | GPIO 출력, 다중 LED, Button Debouncing | [Markdown](technologies/GPIO.md) · [HTML](technologies/GPIO.html) |
| UART | Polling/Interrupt 송수신, 문자열 명령, `printf()` 연결 | [Markdown](technologies/UART.md) · [HTML](technologies/UART.html) |
| DMA | UART RX/TX DMA, Stream/Channel, IRQ와 Callback | [Markdown](technologies/DMA.md) · [HTML](technologies/DMA.html) |
| ADC | CdS Analog Input, 12-bit 단일 변환, Polling Sampling | [Markdown](technologies/ADC.md) · [HTML](technologies/ADC.html) |

## 폴더 구조

```text
blog/
├─ README.md, index.html       전체 문서 목차
├─ fundamentals/              C, HAL, CMSIS 등 공통 기반 지식
├─ technologies/              GPIO, UART, DMA, ADC 등 개별 기술
└─ assets/                    회로도와 문서 이미지
```

전체 프로젝트 진도와 다음 학습 시작점은 루트의 [`LEARNING_PROGRESS.md`](../LEARNING_PROGRESS.md)에서 관리한다.
