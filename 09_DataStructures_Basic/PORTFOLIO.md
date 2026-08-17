# 09 Data Structures Basic

고정 길이 `CommandBuffer`를 구현해 배열 Index, 문자열 널 종료, 경계 검사, 길이 관리의 기초를 학습한다. `\n`은 명령 완료 표식으로 처리하고, 최대 길이를 넘는 문자는 저장하지 않아 Buffer Overflow를 방지한다. 다음 단계에서는 이 구조를 UART DMA의 연속 수신 처리에 연결한다.
