# ADC CdS 프로젝트 진도표

## 현재 단계

| 단계 | 내용 | 상태 |
|---|---|---|
| 1 | `03_UART_DMA` 기반 독립 프로젝트 생성과 내부 이름 변경 | 완료 |
| 2 | PA0를 `ADC1_IN0` Analog Input으로 설정 | 구현·빌드 완료 |
| 3 | ADC1 12-bit, Software Trigger 단일 변환 설정 | 구현·빌드 완료 |
| 4 | `HAL_ADC_Start()`와 Polling 기반 CdS 값 읽기 | 구현·빌드 완료 |
| 5 | 500ms마다 원시값과 환산 mV UART 출력 | 구현·빌드 완료 |
| 6 | Firmware Flash와 Verify | 완료 |
| 7 | CdS 기본 측정값과 UART 출력 확인 | 보드 검증 완료 (`raw=587~588`, `473mV`) |
| 8 | 어두울 때 측정 범위 확인 | 보드 검증 완료 (`raw=3657~3881`, `2.947~3.127V`) |
| 9 | 밝을 때 측정 범위 확인 | 보드 검증 완료 (`raw=252~308`, `203~248mV`) |
| 10 | 측정 범위 기반 LED 임계값 결정과 구현 | 구현·빌드 완료 |
| 11 | LED 단계 표시 Firmware Flash와 Verify | 완료 |
| 12 | 밝기별 LED와 UART `brightness` 출력 확인 | 보드 검증 완료 |

## 빌드 결과

- `03_UART_DMA` Debug 독립 빌드: 완료
- `04_ADC_CdS` Debug 빌드: 완료
- `04_ADC_CdS.elf`: LED 단계 표시 버전 생성 완료
- NUCLEO-F401RE Flash 및 Verify: 완료
- LED 단계 표시 버전 Flash 및 Verify: 완료
- FLASH: 22,776 B / 512 KB
- RAM: 2,544 B / 96 KB

## 다음 확인

1. CdS `VCC → 3.3V`, `GND → GND`, `AO → A0(PA0)` 배선
2. Terminal에서 500ms 주기의 `CdS raw=587~588, voltage=473 mV` 확인 완료
3. 센서를 가렸을 때 `raw=3657~3881`, `2.947~3.127V` 확인 완료
4. 빛을 비췄을 때 `raw=252~308`, `203~248mV` 확인 완료
5. `1200 / 2100 / 3000` 임계값 LED 단계 표시 구현·빌드 완료
6. 밝기별 LED와 UART `brightness` 출력 확인

## 최종 검증

- 휴대폰 플래시를 비춘 밝은 상태에서 `brightness=BRIGHT`와 초록 LED 확인
- 센서를 가린 어두운 상태에서 `brightness=DARK`와 빨강 LED 확인
- ADC 값, UART 출력, LED 단계 표시가 일치함을 실제 NUCLEO-F401RE에서 확인
- 다음 학습: 새 `05_Timer_Sampling` 프로젝트에서 Hardware Timer 기반 주기 Sampling
