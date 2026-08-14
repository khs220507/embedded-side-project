# STM32 UART 학습 기록

**환경:** STM32 NUCLEO-F401RE, C, STM32 HAL, USART2, CMake, ST-LINK

## 구현 내용

- ST-LINK Virtual COM Port를 이용한 USART2 송수신
- NVIC와 USART2 IRQ Handler를 연결한 Interrupt 수신
- 수신 완료 콜백에서 한 바이트씩 문자열 버퍼 구성
- Enter 기반 명령 완성 및 버퍼 범위 검사
- `strcmp()`를 이용한 LED 상태 명령 처리
- `_write()`를 통한 `printf()` UART 출력 연결

## 확인한 동작

- PC 터미널 UART 송신 및 수신
- `led red` 문자열 명령에 따른 빨간색 LED 제어
- Debug 빌드 및 `02_UART.elf` 생성

## 한 줄 요약

STM32 USART2 인터럽트 기반 문자열 명령 수신기를 구현하고 PC 터미널 명령으로 GPIO LED 상태를 제어했다.
