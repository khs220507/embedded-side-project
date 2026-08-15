# STM32 ADC CdS 학습 기록

## 구현 내용

- NUCLEO-F401RE PA0의 `ADC1_IN0`에서 CdS Analog Output 측정
- ADC1 12-bit 단일 변환과 Polling 완료 확인
- `HAL_GetTick()`으로 500ms Sensor Sampling
- 원시값과 3.3V 기준 환산 전압을 USART2 DMA로 출력
- 실측 조도 범위 기반 4단계 LED 표시

## 현재 결과

- 밝은 상태 `raw=252~308`, 어두운 상태 `raw=3657~3881` 측정 완료
- ADC 출력 버전의 Flash·Verify와 기본 UART 측정 완료
- LED 단계 표시 버전의 CMake Debug 빌드와 `04_ADC_CdS.elf` 생성 완료
- LED 단계 표시 버전의 Flash와 실제 LED 동작 확인은 남아 있음

## 다음 확장

새 Firmware를 Flash해 밝기별 LED와 UART `brightness` 출력을 확인한 뒤, Timer 기반 주기 Sampling과 비교한다.
