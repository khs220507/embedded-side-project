# Project 03 — UART DMA Command Controller

NUCLEO-F401RE의 USART2 송수신을 DMA 방식으로 처리하는 학습 프로젝트다. `02_UART`의 LED 문자열 명령 기능을 유지하면서 CPU 대신 DMA1이 메모리와 USART2 사이의 데이터를 옮긴다. 수신 Event와 명령 처리 사이에는 Ring Buffer를 둔다.

## DMA 설정

| 방향 | DMA 자원 | 용도 |
|---|---|---|
| RX | DMA1 Stream5, Channel 4 | USART2 수신 데이터를 32바이트 버퍼로 이동 |
| TX | DMA1 Stream6, Channel 4 | `printf()` 출력 데이터를 USART2로 이동 |

UART 설정은 USART2, PA2/PA3, 115200 bps, 8-N-1이다.

## 핵심 동작

```text
PC에서 문자열과 Enter 입력
→ USART2 RX 요청
→ DMA1 Stream5가 수신 버퍼에 저장
→ IDLE 또는 버퍼 완료 이벤트
→ HAL_UARTEx_RxEventCallback()
→ Ring Buffer에 수신 바이트 저장
→ while (1)에서 Ring Buffer를 읽어 LED 명령 처리
```

`printf()`는 `_write()`에서 `HAL_UART_Transmit_DMA()`를 호출한다. TX 완료 시 `HAL_UART_TxCpltCallback()`이 실행된다.

## 빌드

```powershell
cmake --preset Debug
cmake --build build/Debug
```

Debug 빌드에서 `build/Debug/03_UART_DMA.elf` 생성을 확인했다. Ring Buffer 연결 후 코드는 Debug 빌드를 통과했으며, 이 변경에 대한 STM32 보드 재검증은 추가 확인이 필요하다.

DMA 상세 학습 내용은 `../blog/technologies/DMA.md`, UART 문자열 명령은 `../blog/technologies/UART.md`에서 관리한다.
