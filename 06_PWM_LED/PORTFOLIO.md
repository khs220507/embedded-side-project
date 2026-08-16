# 06 PWM LED

TIM1 PWM과 CdS ADC 값을 연결해 PA8(D7)의 빨간 LED 밝기를 제어하는 STM32 학습 프로젝트다. TIM2는 500ms마다 측정 요청만 만들고, Main Loop가 ADC 값에 따라 TIM1 Channel 1의 Compare 값을 갱신한다.

구현은 완료했으며, 현재 작업 환경에는 Ninja Build Tool이 없어 빌드와 실제 보드 검증은 아직 진행하지 못했다.
