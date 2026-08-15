# Timer Sampling 프로젝트 진도

## 현재 단계

| 단계 | 내용 | 상태 |
|---|---|---|
| 1 | `04_ADC_CdS` 기반 독립 프로젝트 생성 | 완료 |
| 2 | TIM2 Base Timer 및 Update Interrupt 설정 | 구현 완료 |
| 3 | TIM2 IRQ → HAL IRQ Handler → Callback 연결 | 구현 완료 |
| 4 | Callback Flag 기반 Main Loop ADC 처리 | 구현 완료 |
| 5 | `05_Timer_Sampling.elf` Debug 빌드 | 빌드 완료 |
| 6 | NUCLEO-F401RE Flash·Verify | 완료 |
| 7 | 500ms UART 출력과 조도 LED 표시 확인 | 보드 검증 완료 |

## 빌드 결과

- `05_Timer_Sampling` Debug 빌드: 완료
- FLASH: 24,484 B / 512 KB
- RAM: 2,624 B / 96 KB

## 다음 확인

1. `05_Timer_Sampling` 챕터의 Git 기록을 남긴다.
2. 다음 챕터에서 Timer를 다른 주기 작업에도 활용한다.
