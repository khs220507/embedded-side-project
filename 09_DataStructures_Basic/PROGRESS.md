# Data Structures Basic 프로젝트 진도

| 단계 | 내용 | 상태 |
|---:|---|---|
| 1 | 전용 C 학습 프로젝트 생성 | 완료 |
| 2 | 고정 크기 Command Buffer와 길이 관리 구현 | 완료 |
| 3 | 줄바꿈 완료·널 종료 문자 처리 구현 | 완료 |
| 4 | Buffer 경계·Overflow 거부 테스트 구현 | 완료 |
| 5 | ARM GCC 문법·경고 검사 | 완료 |
| 6 | 호스트 CMake 빌드와 CTest 실행 | 완료 — MSVC Build Tools, 1/1 통과 |
| 7 | 배열 기반 Stack의 `push`·`pop` 구현 | 완료 — LIFO·Empty 테스트 통과 |
| 8 | 배열 기반 Queue의 `enqueue`·`dequeue` 구현 | 완료 — FIFO·Empty 테스트 통과 |
| 9 | 원형 버퍼(Ring Buffer) 구현 | 완료 — FIFO·Wrap-around·Full·Empty 경계 테스트 통과 |
| 10 | `03_UART_DMA` 수신 Buffer와 연결 | 예정 |

이 챕터는 STM32 보드 Peripheral을 새로 제어하지 않는 C 기초 학습이므로 보드 검증 대상이 아니다. 현재 링버퍼는 `int` 자료를 저장하는 호스트 C 구현이며, 다음 단계에서 UART DMA 수신 바이트 저장 구조로 확장한다.
