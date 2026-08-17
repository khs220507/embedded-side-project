# C 자료구조

## DS-01. 배열·문자열 Command Buffer

### 구현 목표

`09_DataStructures_Basic`에서 고정 길이 배열로 UART 명령과 같은 문자열을 안전하게 모은다. 이 단계는 다음 `03_UART_DMA` 수신 Buffer 확장의 기반이다.

### 핵심 코드

```c
typedef struct
{
    char data[COMMAND_BUFFER_CAPACITY];
    size_t length;
} CommandBuffer;
```

`command_buffer_append()`는 일반 문자를 배열에 추가하고 항상 마지막에 `\0`을 둔다. `\n`은 저장하지 않고 명령 완료 상태를 반환한다. 길이가 `COMMAND_BUFFER_CAPACITY - 1`이면 문자를 더 저장하지 않고 `COMMAND_BUFFER_FULL`을 반환한다.

### 코드 설명

- `data`의 마지막 칸은 C 문자열 끝을 뜻하는 `\0` 전용이다. 따라서 용량이 16이면 실제 문자는 최대 15개다.
- `length`를 별도로 관리하면 매번 문자열 전체를 검색하지 않고 다음 저장 위치를 알 수 있다.
- Overflow를 감지한 뒤 기존 내용을 유지하므로 배열 경계를 벗어나 메모리를 덮어쓰지 않는다.

### 실행 결과

- `arm-none-eabi-gcc`로 `-Wall -Wextra -Wpedantic -Werror` 조건의 문법·경고 검사를 통과했다.
- MSVC Build Tools에서 호스트 CMake 빌드와 CTest를 실행했고, 1개 테스트가 통과했다.
- STM32 보드 검증은 아직 하지 않았다. 이 챕터는 PC C 코드이며, 다음 단계에서 UART DMA 수신 흐름과 연결한다.

### 배운 점

임베디드에서는 고정 배열의 경계 검사와 `\0` 예약이 문자열 처리의 기본 안전 장치다. 이후 원형 버퍼는 이 배열·Index·길이 관리 개념을 확장한다.

### 한 줄 정리

고정 길이 Command Buffer는 배열 범위를 지키면서 한 줄 명령을 모으는 가장 단순한 자료구조다.
