# Project 06 - PWM LED

`05_Timer_Sampling`을 기반으로 TIM1 Channel 1의 PWM 출력으로 빨간 LED 밝기를 제어하는 프로젝트다.

## 구현 내용

- PA8(D7)의 빨간 LED를 `TIM1_CH1` PWM 출력으로 사용
- TIM1 Clock 84MHz, Prescaler `83`, Period `999`로 1kHz PWM 생성
- TIM2 Update Interrupt가 500ms마다 CdS ADC 측정을 요청
- CdS ADC 원시값 `0~4095`를 PWM Compare 값 `0~1000`으로 변환
- 밝은 환경에서는 낮은 Duty Cycle, 어두운 환경에서는 높은 Duty Cycle로 빨간 LED 밝기 변경
- USART2 DMA 출력으로 ADC 값과 PWM Compare 값을 표시

## 동작 흐름

```text
TIM2 Update Interrupt (500ms)
  → Main Loop가 CdS ADC 측정
  → ADC raw 값을 TIM1 Compare 값으로 변환
  → TIM1_CH1(PA8) Duty Cycle 갱신
  → 빨간 LED 밝기 변경
```

## 빌드와 보드 검증

- 구현 완료
- Debug 빌드 완료: FLASH 26,716 B / 512 KB, RAM 2,696 B / 96 KB
- NUCLEO-F401RE Flash·Verify·Reset 완료
- 보드 검증 완료: PA8(D7) 빨간 LED의 밝기 변화와 UART PWM Compare 값 출력을 밝음·어두움 조건에서 함께 확인
