# STM32 UART DMA 학습 기록

**환경:** STM32 NUCLEO-F401RE, C, STM32 HAL, USART2, DMA1, CMake, ST-LINK

## 구현 내용

- USART2 RX와 DMA1 Stream5 Channel 4 연결
- USART2 TX와 DMA1 Stream6 Channel 4 연결
- `HAL_UARTEx_ReceiveToIdle_DMA()` 기반 가변 길이 수신
- 수신 이벤트 Callback과 기존 문자열 LED 명령 처리 연결
- `_write()`에서 `HAL_UART_Transmit_DMA()`를 사용하는 `printf()` 송신
- DMA Stream IRQ Handler와 UART Handle 연결

## 확인한 동작

- `02_UART`와 `03_UART_DMA`의 독립 Debug 빌드 성공
- `03_UART_DMA.elf` 생성 성공
- STM32CubeProgrammer Flash 기록 및 Verify
- 실제 보드의 DMA 송수신과 문자열 LED 명령 제어

## 한 줄 요약

STM32 USART2와 DMA1을 연결해 데이터 이동을 DMA가 담당하고 CPU가 완성된 문자열 명령을 처리하도록 구현했다.
