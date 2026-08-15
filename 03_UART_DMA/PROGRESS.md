# UART DMA 프로젝트 진도표

마지막 업데이트: 2026-08-15

| 단계 | 학습 내용 | 상태 |
|---:|---|---|
| 1 | `02_UART`를 독립 프로젝트로 복제 | 완료 |
| 2 | USART2 RX: DMA1 Stream5, Channel 4 설정 | 구현·빌드 완료 |
| 3 | USART2 TX: DMA1 Stream6, Channel 4 설정 | 구현·빌드 완료 |
| 4 | `HAL_UARTEx_ReceiveToIdle_DMA()` 수신 | 구현·빌드 완료 |
| 5 | `HAL_UARTEx_RxEventCallback()`에서 수신 데이터 처리 | 구현·빌드 완료 |
| 6 | `_write()`와 `HAL_UART_Transmit_DMA()` 연결 | 구현·빌드 완료 |
| 7 | Polling / Interrupt / DMA 비교 | 문서화 완료 |
| 8 | 실제 보드에서 DMA 송수신과 LED 명령 확인 | 완료 |

## 현재 빌드

- `02_UART` Debug 빌드 성공
- `03_UART_DMA` Debug 빌드 성공
- `03_UART_DMA.elf` 생성 성공
- RAM 2.51%, FLASH 3.86% 사용

STM32CubeProgrammer로 Flash 기록과 Verify를 완료했고, 실제 보드에서 UART DMA 송수신과 LED 명령 동작을 확인했다.
