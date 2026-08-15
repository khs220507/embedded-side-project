# STM32 CMSIS: `__WFI()`로 Interrupt 대기

## 기본 개념

CMSIS(Cortex Microcontroller Software Interface Standard)는 ARM Cortex-M Processor를 공통된 이름과 Interface로 사용할 수 있게 하는 표준 계층이다. STM32 HAL이 GPIO, UART, DMA 같은 STM32 주변장치를 다룬다면, CMSIS-Core는 Cortex-M CPU Register와 명령을 다룬다.

```text
Application Code
→ STM32 HAL: UART, DMA 같은 주변장치 제어
→ CMSIS-Core: Cortex-M CPU 기능과 명령 사용
→ Cortex-M4 Processor
```

## 구현 목표

`03_UART_DMA`의 `_write()`에서 DMA 송신이 끝날 때까지 CPU가 빈 반복문을 계속 실행하지 않도록 `__WFI()`를 사용한다.

## 핵심 코드

```c
while (uartDmaTxComplete == 0U)
{
    __WFI();
}
```

GCC용 CMSIS Header에는 `__WFI()`가 다음과 같이 정의되어 있다.

```c
#define __WFI() __ASM volatile ("wfi":::"memory")
```

`__WFI()`는 일반 C 표준 함수나 STM32 HAL 함수가 아니다. CMSIS가 제공하는 Compiler Intrinsic Macro이며, ARM의 `WFI`(Wait For Interrupt) 명령을 생성한다.

## 코드 설명

`uartDmaTxComplete`가 `0U`이면 DMA 송신이 아직 끝나지 않은 상태다. CPU는 `__WFI()`를 실행하고 Interrupt가 발생할 때까지 대기한다.

```text
HAL_UART_Transmit_DMA()
→ uartDmaTxComplete == 0U
→ CPU가 __WFI()로 대기
→ DMA 또는 USART2 Interrupt 발생
→ CPU 실행 재개
→ HAL_UART_TxCpltCallback()
→ uartDmaTxComplete = 1U
→ while 종료
```

Interrupt가 발생해 CPU가 깨어나도 그 Interrupt가 반드시 UART TX 완료라는 보장은 없다. 그래서 `__WFI()` 다음에 바로 완료로 판단하지 않고 `while` 조건으로 Flag를 다시 확인한다.

## 주의 사항

- DMA와 USART2 IRQ가 활성화되어 있어야 TX 완료 Callback이 실행된다.
- Interrupt가 비활성화되어 완료 Flag가 바뀌지 않으면 `_write()`가 계속 대기한다.
- 현재 `_write()`는 CPU가 대기하는 동안 불필요한 반복 연산은 줄이지만, 호출자는 송신 완료까지 반환받지 못하는 동기식 구조다.
- 완전한 Non-blocking 송신에는 별도의 TX Queue나 Ring Buffer가 필요하다.

## 실행 결과

`__WFI()`가 포함된 `03_UART_DMA`의 Debug 빌드, Flash 기록과 Verify를 완료했다. 실제 보드에서 DMA 송신 완료 Callback이 Flag를 변경하고 `_write()`의 대기가 정상적으로 해제되어 UART 출력이 이어지는 것을 확인했다.

## 배운 점

- `__WFI()`는 HAL 함수가 아니라 CMSIS-Core가 제공하는 CPU Intrinsic이다.
- `WFI`는 Interrupt가 발생할 때까지 Processor 실행을 대기시킨다.
- CPU가 깨어난 뒤에는 원하는 작업이 완료됐는지 Flag를 다시 검사해야 한다.
- DMA 전송과 `__WFI()` 대기는 IRQ와 Callback이 정상적으로 연결되어 있어야 동작한다.

## 한 줄 정리

> `__WFI()`는 DMA 송신 완료 Flag를 기다리는 동안 Cortex-M4가 불필요한 반복 연산을 하지 않고 Interrupt를 기다리게 한다.
