# Project 02 — UART Command Controller

NUCLEO-F401RE의 USART2와 ST-LINK Virtual COM Port를 이용해 PC 터미널 명령으로 네 개의 LED 상태를 제어하는 학습 프로젝트다.

## 통신 설정

| 항목 | 설정 |
|---|---|
| UART | USART2 |
| TX / RX | PA2 / PA3 |
| Baud rate | 115200 bps |
| Frame | 8-N-1 |
| Flow control | None |

## 지원 명령

| 명령 | 결과 |
|---|---|
| `led green` | 초록색 LED, IDLE |
| `led blue` | 파란색 LED, RUNNING |
| `led yellow` | 노란색 LED, WARNING |
| `led red` | 빨간색 LED, ERROR |
| `status` | 현재 LED 상태 출력 |
| `help` | 명령 목록 출력 |

명령 뒤에 Enter를 입력해야 한다. 한 글자 명령 `g`, `b`, `y`, `r`도 지원한다.

## 핵심 구현

```text
USART2 인터럽트 수신
→ 한 바이트씩 문자열 버퍼에 저장
→ Enter에서 명령 완성
→ strcmp()로 명령 비교
→ LED 상태 변경
→ printf()로 결과 출력
```

## 빌드

```powershell
cmake --preset Debug
cmake --build build/Debug
```

빌드 결과는 `build/Debug/02_UART.elf`에 생성된다.

상세 학습 내용은 `../blog/UART.md`에서 관리한다.
