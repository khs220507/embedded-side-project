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

## DS-02. 원형 버퍼(Ring Buffer)

### 기본 개념

일반 Queue는 데이터를 꺼낸 뒤 앞쪽 빈 공간을 다시 사용하려면 배열을 이동하거나 별도 정리가 필요하다. Ring Buffer는 배열의 마지막 Index 다음을 0번 Index로 연결해, `head`와 `tail`이 배열을 순환하도록 만든 고정 크기 FIFO 구조다.

### 구현 목표

- `head`는 다음에 꺼낼 데이터의 위치를 가리킨다.
- `tail`은 다음에 저장할 위치를 가리킨다.
- `count`로 Empty와 Full을 구분한다.
- 데이터를 꺼낸 뒤 생긴 앞쪽 빈 칸을 `tail`이 다시 사용한다.

### 핵심 코드

```c
#define RING_BUFFER_CAPACITY 4U

typedef struct
{
    int data[RING_BUFFER_CAPACITY];
    size_t head;
    size_t tail;
    size_t count;
} RingBuffer;
```

```c
buffer->data[buffer->tail] = value;
buffer->tail = (buffer->tail + 1U) % RING_BUFFER_CAPACITY;
buffer->count++;
```

`ring_buffer_enqueue()`는 먼저 `count`를 검사해 Full이면 저장하지 않는다. 저장 후 `tail`을 1 증가시키고 `% RING_BUFFER_CAPACITY`로 배열 끝에서 0으로 되돌린다. `ring_buffer_dequeue()`도 같은 방식으로 `head`를 순환시키며 가장 오래된 값을 반환한다.

### 동작 흐름

```text
enqueue: data[tail] 저장 → tail 순환 → count 증가
dequeue: data[head] 반환 → head 순환 → count 감소
```

예를 들어 용량 4에서 `10, 20, 30`을 저장한 뒤 `10`을 꺼내면, `tail`은 3에서 0으로 돌아가 비어 있는 첫 칸을 다시 사용한다. 이때 데이터 순서는 `20, 30, ...`로 유지되므로 배열 Index의 물리적 순서와 논리적 FIFO 순서는 다를 수 있다.

### 실행 결과

- `head`, `tail`, `count`가 초기화되는지 확인했다.
- `10, 20, 30, 40, 50`의 FIFO 순서와 `tail`의 Wrap-around를 확인했다.
- Full 상태에서 새 데이터를 거부하고 Empty 상태에서 꺼내기를 거부하는 테스트를 통과했다.
- 호스트 CMake·CTest는 1/1 통과했으며, STM32 실제 보드 검증은 하지 않았다.

### 배운 점

`count`를 함께 두면 `head == tail`만으로는 구분하기 어려운 Empty와 Full을 명확하게 판단할 수 있다. 현재 구현은 단일 실행 흐름의 일반 C 구조이며, UART DMA Interrupt와 Main Loop가 동시에 접근하는 단계에서는 공유 Index의 원자성, `volatile` 필요성, 임계 구역을 별도로 검토해야 한다.

### 한 줄 정리

Ring Buffer는 배열을 이동하지 않고 `head`와 `tail`을 순환시켜 고정 메모리에서 FIFO를 구현하는 자료구조다.
