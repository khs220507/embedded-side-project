# PWM LED 프로젝트 진도

| 단계 | 내용 | 상태 |
|---|---|---|
| 1 | `05_Timer_Sampling` 기반 독립 프로젝트 생성 | 완료 |
| 2 | TIM1 Channel 1과 PA8(D7) PWM 출력 설정 | 구현 완료 |
| 3 | CdS ADC 값을 PWM Duty Cycle로 변환 | 구현 완료 |
| 4 | UART로 ADC/PWM 값 출력 | 구현 완료 |
| 5 | `06_PWM_LED.elf` Debug 빌드 | 빌드 완료 |
| 6 | NUCLEO-F401RE Flash·Verify | 완료 |
| 7 | 밝기 변화 보드 검증 | 보드 검증 완료 |

## 보드 검증 기록

PA8(D7)의 빨간 LED와 GND 배선을 확인했고, 밝음·어두움 조건에서 LED 밝기와 UART의 PWM Compare 값 출력이 함께 바뀌는 것을 NUCLEO-F401RE에서 확인했다.

## 다음 학습

`07_I2C` 챕터에서 I2C Peripheral 또는 Sensor 통신을 시작한다.
