# Project 05 - Timer Sampling

`04_ADC_CdS`의 CdS 측정 프로젝트를 기반으로, `HAL_GetTick()` 비교 대신 TIM2 Update Interrupt가 500ms마다 측정 시점을 만드는 프로젝트다.

## 현재 구현

- TIM2 Base Timer: Prescaler `8399`, Period `4999`
- APB1 Timer Clock 84MHz 기준 500ms Update Interrupt
- `HAL_TIM_PeriodElapsedCallback()`에서 `timerSampleRequested` Flag 설정
- Main Loop에서 ADC Polling, 조도 단계 LED, USART2 DMA `printf()` 처리
- 기존 CdS 및 4색 LED 배선 변경 없음

## 동작 흐름

```text
TIM2 Update Interrupt (500ms)
  -> timerSampleRequested = 1
  -> Main Loop
  -> ADC1 PA0 측정 / LED 갱신 / UART 출력
```

Interrupt 안에는 시간이 걸릴 수 있는 ADC Polling과 `printf()`를 넣지 않았다.

## 빌드

```powershell
cmake --preset Debug
cmake --build --preset Debug -j
```

Debug 빌드로 `build/Debug/05_Timer_Sampling.elf` 생성까지 확인했고, NUCLEO-F401RE Flash·Verify와 UART 약 500ms 출력·조도 LED 동작도 확인했다.
