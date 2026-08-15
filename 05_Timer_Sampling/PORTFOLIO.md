# 05 Timer Sampling

TIM2 Hardware Timer를 이용해 CdS ADC 측정 주기를 만드는 STM32 학습 프로젝트다.

Timer Update Interrupt는 500ms마다 Flag만 설정한다. ADC Polling, 4단계 LED 표시, USART2 DMA 기반 UART 출력은 Main Loop에서 실행해 Interrupt를 짧게 유지했다.

구현·Debug 빌드·NUCLEO-F401RE Flash·Verify를 완료했고, UART 약 500ms 출력과 조도 LED 동작도 실제 보드에서 확인했다.
