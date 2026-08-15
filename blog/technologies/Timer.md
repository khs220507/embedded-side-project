# STM32 Timer: TIM2 Interrupt 기반 ADC Sampling

## 기본 개념

Timer는 MCU 내부 Clock을 세어 정해진 시간마다 Event를 만드는 주변장치다. `05_Timer_Sampling`에서는 TIM2 Update Event를 Interrupt로 받아 CdS 측정 시점을 500ms마다 만든다.

APB1 Timer Clock은 84MHz다. Prescaler를 `8399`로 설정하면 `84MHz ÷ (8399 + 1) = 10kHz`가 되어 Counter는 100µs마다 하나씩 증가한다. Period가 `4999`이면 5,000번 세므로 `100µs × 5000 = 500ms`마다 Update Event가 발생한다.

## 구현 목표

- 기존 CdS·LED·USART2 DMA 회로를 그대로 사용한다.
- TIM2를 500ms Update Interrupt로 설정한다.
- Interrupt에서는 측정 요청 Flag만 설정한다.
- Main Loop가 Flag를 확인해 ADC 측정, LED 표시, UART 출력을 실행한다.

## 핵심 코드

### TIM2 초기화와 시작

```c
htim2.Instance = TIM2;
htim2.Init.Prescaler = 8399;
htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
htim2.Init.Period = 4999;
htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

HAL_TIM_Base_Init(&htim2);
HAL_TIM_Base_Start_IT(&htim2);
```

`HAL_TIM_Base_Init()`은 TIM2 설정을 적용하고, `HAL_TIM_Base_Start_IT()`은 Counter와 Update Interrupt를 함께 시작한다.

### IRQ와 Callback

```c
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        timerSampleRequested = 1U;
    }
}
```

`TIM2_IRQHandler()`는 하드웨어 Interrupt를 HAL로 넘긴다. HAL은 Update Event를 처리한 뒤 `HAL_TIM_PeriodElapsedCallback()`을 호출한다. `timerSampleRequested`는 Interrupt와 Main Loop가 함께 접근하므로 `volatile`로 선언했다.

### Main Loop의 측정 처리

```c
if (timerSampleRequested != 0U)
{
    timerSampleRequested = 0U;
    CdS_ProcessSample();
}
```

`CdS_ProcessSample()` 안에서는 기존처럼 ADC Polling, 조도 단계 LED 갱신, UART `printf()`를 수행한다.

## 동작 흐름

`TIM2 Counter` → `500ms Update Event` → `TIM2_IRQHandler()` → `HAL_TIM_IRQHandler()` → Callback에서 Flag 설정 → Main Loop → ADC·LED·UART 처리

### 누가 실행하는가

```text
main()                                 MCU Hardware / HAL
────────────────────────────────────────────────────────────────
HAL_TIM_Base_Start_IT(&htim2)
  └─ TIM2 Counter 시작

while (1) 계속 실행                    TIM2가 500ms를 셈
버튼·UART 등 다른 작업 수행                  │
                                           ▼
                                    Update Interrupt 발생
                                           │
                                    TIM2_IRQHandler()
                                           │
                                    HAL_TIM_IRQHandler()
                                           │
                                    HAL_TIM_PeriodElapsedCallback()
                                           │
                                    timerSampleRequested = 1
                                           │
                  ◀──── Interrupt 종료: 원래 while (1) 위치로 복귀

if (timerSampleRequested != 0U)
  └─ Flag를 0으로 지움
  └─ CdS_ProcessSample()
       └─ ADC 측정 → LED 갱신 → UART 출력
```

`TIM2_IRQHandler()`는 Main Loop가 직접 부르는 함수가 아니다. 500ms Update Event가 발생하면 MCU가 현재 Main Loop를 잠깐 멈추고 자동으로 실행한다. Callback까지 끝나면 MCU는 멈췄던 Main Loop 자리로 돌아온다.

Interrupt 안에서 `printf()`나 ADC Polling을 실행하지 않는다. 이 작업들은 시간이 걸릴 수 있으므로, 짧은 Callback은 “지금 측정할 차례”만 Main Loop에 알린다.

## 배운 점

- `Prescaler`와 `Period`는 레지스터 값이므로 실제 분주·횟수는 각각 `+1`을 적용해 계산한다.
- Hardware Timer는 Main Loop가 바쁘더라도 독립적으로 주기 Event를 만든다.
- IRQ Handler는 HAL IRQ Handler로 전달하고, 실제 사용자 동작은 Callback에 작성할 수 있다.
- Interrupt와 Main Loop가 공유하는 Flag에는 `volatile`이 필요하다.

## 한 줄 정리

**TIM2가 500ms마다 Interrupt로 측정 요청 Flag를 세우고, Main Loop가 안전하게 CdS ADC 측정·LED 표시·UART 출력을 처리한다.**

## 검증 상태

- 구현 완료: TIM2 Interrupt 기반 Sampling 코드
- Debug 빌드 완료: `05_Timer_Sampling.elf`
- Flash·Verify: NUCLEO-F401RE에서 완료
- 실제 동작 검증: UART 약 500ms 출력과 밝음·어두움 LED 표시 확인 완료
