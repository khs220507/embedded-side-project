# C 언어 학습 기록

## GPIO 포트를 함수에 전달할 때 사용한 포인터

### 사용 목적

여러 LED가 서로 다른 GPIO 포트에 연결되어 있어도 하나의 `LED_On()` 함수로 제어하기 위해 GPIO 포트를 매개변수로 전달했다.

### 핵심 코드

```c
static void LED_On(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    LED_AllOff();
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}
```

```c
LED_On(GPIOB, GREEN_LED_Pin);
LED_On(GPIOA, RED_LED_Pin);
```

### 포인터가 사용된 부분

```c
GPIO_TypeDef *GPIOx
```

`GPIOx`는 `GPIO_TypeDef` 구조체를 가리키는 포인터다. STM32의 `GPIOA`, `GPIOB`는 각 GPIO 주변장치의 레지스터가 배치된 메모리 주소를 나타내므로, 함수는 이 포인터를 통해 어느 GPIO 포트를 제어할지 전달받는다.

예를 들어 다음 호출은 GPIOB와 초록색 LED의 핀 값을 전달한다.

```c
LED_On(GPIOB, GREEN_LED_Pin);
```

함수 내부에서는 전달받은 값이 다음과 같이 사용된다.

```c
HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
```

이 호출에서 `GPIOx`는 `GPIOB`, `GPIO_Pin`은 `GREEN_LED_Pin`에 해당한다.

### 포인터와 일반 값의 차이

| 매개변수 | 전달하는 내용 | 전달 방식 |
|---|---|---|
| `GPIO_TypeDef *GPIOx` | GPIO 주변장치 레지스터의 주소 | 포인터 |
| `uint16_t GPIO_Pin` | 제어할 핀의 비트값 | 값 |

`GPIOx`는 하드웨어 레지스터가 있는 위치를 가리켜야 하므로 포인터로 받고, `GPIO_Pin`은 핀을 선택하는 작은 정수형 비트값이므로 값으로 받는다.

### 배운 점

- 포인터는 데이터뿐 아니라 하드웨어 레지스터의 위치를 전달할 때도 사용한다.
- GPIO 포트를 포인터 매개변수로 받으면 하나의 함수로 GPIOA, GPIOB 등 여러 포트를 제어할 수 있다.
- 함수 선언의 `*`는 해당 매개변수가 주소를 저장하는 포인터라는 뜻이다.

### 한 줄 정리

`GPIO_TypeDef *GPIOx` 포인터로 제어할 GPIO 포트의 주소를 전달하여, 하나의 `LED_On()` 함수를 여러 GPIO 포트에 재사용했다.

---

## 구조체로 GPIO 레지스터 표현하기

### 확인한 코드

STM32F401의 GPIO 구조체는 `stm32f401xe.h`에 다음과 같이 정의되어 있다.

```c
typedef struct
{
    __IO uint32_t MODER;
    __IO uint32_t OTYPER;
    __IO uint32_t OSPEEDR;
    __IO uint32_t PUPDR;
    __IO uint32_t IDR;
    __IO uint32_t ODR;
    __IO uint32_t BSRR;
    __IO uint32_t LCKR;
    __IO uint32_t AFR[2];
} GPIO_TypeDef;
```

### `typedef struct`의 의미

구조체는 서로 관련된 여러 데이터를 하나의 자료형으로 묶는 C 언어 문법이다. 위 선언은 GPIO에 필요한 여러 레지스터를 하나로 묶고, 그 구조체 자료형에 `GPIO_TypeDef`라는 이름을 붙인다.

```c
GPIO_TypeDef gpio;
```

일반적인 C 코드라면 위와 같이 구조체 변수를 만들 수 있다. STM32에서는 새로운 변수를 만드는 대신 실제 GPIO 레지스터가 존재하는 메모리 주소를 `GPIO_TypeDef` 구조체 형태로 바라본다.

### 구조체 멤버와 하드웨어 레지스터

`GPIO_TypeDef`의 멤버는 단순한 프로그램 데이터가 아니라 MCU 내부의 실제 GPIO 레지스터 배치와 같은 순서로 선언되어 있다.

| 멤버 | 주소 오프셋 | 역할 |
|---|---:|---|
| `MODER` | `0x00` | 핀의 입력·출력·대체 기능 모드 설정 |
| `OTYPER` | `0x04` | Push-Pull·Open-Drain 출력 방식 설정 |
| `OSPEEDR` | `0x08` | 출력 속도 설정 |
| `PUPDR` | `0x0C` | Pull-up·Pull-down 설정 |
| `IDR` | `0x10` | 입력 상태 읽기 |
| `ODR` | `0x14` | 출력 상태 읽기 및 변경 |
| `BSRR` | `0x18` | 핀 출력의 SET·RESET |
| `LCKR` | `0x1C` | GPIO 설정 잠금 |
| `AFR[2]` | `0x20`, `0x24` | UART 등 대체 기능 선택 |

각 멤버가 `uint32_t`이므로 기본적으로 4바이트 간격으로 배치된다. 이 배치가 STM32 데이터시트에 정의된 GPIO 레지스터 주소 간격과 일치한다.

### `__IO`의 의미

```c
__IO uint32_t MODER;
```

STM32 CMSIS에서 `__IO`는 읽기와 쓰기가 가능한 하드웨어 레지스터임을 나타내며, 내부적으로 `volatile`이 사용된다. 하드웨어에 의해 값이 바뀔 수 있으므로 컴파일러가 레지스터 접근을 임의로 제거하거나 캐시하지 못하게 한다.

### GPIO 포트와 구조체의 연결

```c
#define GPIOA ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB ((GPIO_TypeDef *) GPIOB_BASE)
```

`GPIOA_BASE`와 `GPIOB_BASE`는 각 GPIO 포트의 시작 주소다. 해당 주소를 `GPIO_TypeDef *`로 변환하면 구조체 멤버를 통해 GPIO 레지스터에 접근할 수 있다.

개념적으로 다음 두 표현은 연결되어 있다.

```c
GPIOB->ODR
```

```text
GPIOB 시작 주소 + ODR의 오프셋 0x14
```

여기서 `->` 연산자는 구조체 포인터가 가리키는 대상의 멤버에 접근할 때 사용한다.

### 배운 점

- 구조체는 관련된 데이터를 하나의 자료형으로 묶는다.
- STM32에서는 구조체의 메모리 배치를 하드웨어 레지스터 배치와 일치시킨다.
- GPIO 포트의 시작 주소를 구조체 포인터로 변환하면 멤버 이름으로 레지스터에 접근할 수 있다.
- 구조체 포인터의 멤버에는 `->` 연산자로 접근한다.
- 하드웨어 레지스터에는 컴파일러 최적화를 제한하기 위해 `volatile` 성격의 `__IO`가 사용된다.

### 한 줄 정리

`GPIO_TypeDef`는 GPIO 레지스터의 실제 메모리 배치를 C 구조체로 표현하여, 주소 계산 대신 의미 있는 멤버 이름으로 하드웨어를 제어할 수 있게 한다.

---

## `enum`과 `switch`로 상태 머신 만들기

LED 상태를 의미 없는 숫자 대신 열거형으로 정의했다.

```c
typedef enum
{
    LED_STATE_IDLE,
    LED_STATE_RUNNING,
    LED_STATE_WARNING,
    LED_STATE_ERROR,
    LED_STATE_COUNT
} LED_State;
```

열거형 값은 선언 순서에 따라 `IDLE=0`, `RUNNING=1`, `WARNING=2`, `ERROR=3`이 된다. `LED_STATE_COUNT=4`는 실제 상태가 아니라 전체 상태 수를 나타낸다.

현재 상태는 다음 변수에 보관하며 시작값은 `IDLE`이다.

```c
static LED_State currentState = LED_STATE_IDLE;
```

버튼이 한 번 눌리면 나머지 연산으로 다음 상태를 계산한다.

```c
currentState = (LED_State)((currentState + 1) % LED_STATE_COUNT);
```

계산 결과는 다음과 같이 순환한다.

```text
0(IDLE)    → (0 + 1) % 4 = 1(RUNNING)
1(RUNNING) → (1 + 1) % 4 = 2(WARNING)
2(WARNING) → (2 + 1) % 4 = 3(ERROR)
3(ERROR)   → (3 + 1) % 4 = 0(IDLE)
```

`%` 연산 덕분에 마지막 상태 다음 값이 배열 범위 밖인 4가 되지 않고 처음 상태인 0으로 돌아온다. 계산 중 만들어진 정수값은 `(LED_State)`로 열거형 자료형에 맞춘다.

### 상태별 동작 분리

`LED_ShowState()`에서는 `switch`로 현재 상태에 해당하는 LED를 선택한다.

```c
switch (state)
{
    case LED_STATE_IDLE:
        LED_On(GREEN_LED_GPIO_Port, GREEN_LED_Pin);
        break;

    case LED_STATE_RUNNING:
        LED_On(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
        break;

    case LED_STATE_WARNING:
        LED_On(YELLOW_LED_GPIO_Port, YELLOW_LED_Pin);
        break;

    case LED_STATE_ERROR:
        LED_On(RED_LED_GPIO_Port, RED_LED_Pin);
        break;

    default:
        LED_AllOff();
        break;
}
```

각 `break`는 선택된 동작을 실행한 뒤 `switch`를 종료한다. `default`는 예상하지 못한 상태값이 들어왔을 때 모든 LED를 끄는 안전 처리다. 상태값과 실제 GPIO 출력을 함수로 분리했기 때문에 상태 순서나 표시 방법을 독립적으로 수정할 수 있다.

### 한 줄 정리

`enum`으로 상태의 의미와 개수를 정의하고 `switch`로 상태별 동작을 분리하여 버튼 기반 LED 상태 머신을 구성했다.

---

## `uint8_t`, `uint16_t`, `uint32_t` 고정 폭 정수형

### 구현 목표

UART로 전송할 문자열, GPIO Pin 값, 버튼 디바운싱 시간을 저장하면서 데이터의 용도에 맞는 크기의 정수 자료형을 사용한다.

### 핵심 코드

현재 코드에서는 다음 세 가지 자료형을 사용한다.

```c
uint8_t message[] = "Hello STM32!\r\n";

static void LED_On(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

uint32_t lastDebounceTime = 0U;
```

### 고정 폭 정수형이란?

`uint8_t`, `uint16_t`, `uint32_t`는 C 표준 헤더인 `stdint.h`에 정의된 정수 자료형이다. 이름에 자료형의 성격과 bit 수가 포함되어 있다.

```text
u    → unsigned, 음수 없음
int  → integer, 정수
8    → 8bit
_t   → type, 자료형 이름
```

따라서 `uint8_t`는 **음수가 없는 8bit 정수 자료형**을 의미한다.

| 자료형 | 크기 | 표현 범위 | 현재 코드의 용도 |
|---|---:|---:|---|
| `uint8_t` | 1byte | 0 ~ 255 | UART로 전송할 byte 배열 |
| `uint16_t` | 2byte | 0 ~ 65,535 | GPIO Pin 선택 bit 값 |
| `uint32_t` | 4byte | 0 ~ 4,294,967,295 | `HAL_GetTick()`의 밀리초 시간 |

일반적인 `int`나 `unsigned int`는 컴파일 환경에 따라 크기가 달라질 수 있다. 고정 폭 정수형은 크기가 이름에 명확히 나타나므로 레지스터, 통신 데이터, 시간값처럼 데이터 크기가 중요한 임베디드 코드에서 자주 사용한다.

### UART 데이터에 `uint8_t`를 사용하는 이유

```c
uint8_t message[] = "Hello STM32!\r\n";
```

UART는 데이터를 byte 단위로 전송한다. `uint8_t` 하나의 크기는 1byte이므로 문자 데이터를 순서대로 저장하고 전송하기에 적합하다.

`message`는 배열이지만 함수 인자로 전달될 때 첫 번째 원소의 주소로 사용된다.

```c
HAL_UART_Transmit(&huart2,
                  message,
                  sizeof(message) - 1U,
                  HAL_MAX_DELAY);
```

현재 HAL의 `HAL_UART_Transmit()`도 전송 데이터의 주소를 다음 형식으로 받는다.

```c
const uint8_t *pData
```

### GPIO Pin 값에 `uint16_t`를 사용하는 이유

```c
static void LED_On(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
```

STM32 GPIO Port 하나에는 Pin 0부터 Pin 15까지 총 16개의 Pin이 있다. HAL은 각 Pin을 16bit 안의 서로 다른 bit로 표현하므로 GPIO Pin 값에는 `uint16_t`를 사용한다.

```text
GPIO_PIN_0  → 0000 0000 0000 0001
GPIO_PIN_5  → 0000 0000 0010 0000
GPIO_PIN_15 → 1000 0000 0000 0000
```

같은 Port의 여러 Pin을 `|`로 묶을 수 있는 것도 각각의 Pin이 서로 다른 bit를 사용하기 때문이다.

### 시간값에 `uint32_t`를 사용하는 이유

```c
uint32_t lastDebounceTime = 0U;
```

`HAL_GetTick()`은 시스템 시작 후 흐른 시간을 밀리초 단위의 `uint32_t` 값으로 반환한다. 따라서 이전 시각을 저장하는 `lastDebounceTime`도 같은 `uint32_t`로 선언했다.

```c
if ((HAL_GetTick() - lastDebounceTime) >= BUTTON_DEBOUNCE_MS)
```

두 값의 자료형을 맞추면 시간 차이를 같은 범위 안에서 계산할 수 있다.

### 숫자 뒤의 `U`

```c
uint32_t lastDebounceTime = 0U;
sizeof(message) - 1U
```

숫자 뒤의 `U`는 해당 정수 상수를 `unsigned int`로 취급하라는 접미사다.

```c
0   /* int */
0U  /* unsigned int */
```

현재 코드처럼 음수가 없는 값과 계산할 때 `U`를 붙이면 signed 정수와 unsigned 정수가 섞이는 상황을 줄이고, 값이 음수가 될 의도가 없다는 점을 코드에 나타낼 수 있다.

### `sizeof(message) - 1U`의 의미

```c
uint8_t message[] = "Hello STM32!\r\n";
```

C 문자열 배열의 마지막에는 문자열의 끝을 나타내는 null 문자 `\0`이 자동으로 들어간다. `sizeof(message)`는 이 null 문자까지 포함한 배열 전체 크기를 byte 단위로 계산한다.

UART로 화면에 표시할 문자만 보내기 위해 마지막 `\0` 한 byte를 제외한다.

```c
sizeof(message) - 1U
```

### 배운 점

- `uint8_t`, `uint16_t`, `uint32_t`는 크기가 명확한 unsigned 정수 자료형이다.
- UART 데이터처럼 byte 단위인 값에는 `uint8_t`가 적합하다.
- GPIO Pin bit 값에는 16bit인 `uint16_t`를 사용한다.
- `HAL_GetTick()`의 반환값과 시간 저장 변수에는 `uint32_t`를 사용한다.
- 숫자 뒤의 `U`는 unsigned 정수 상수를 의미한다.
- `sizeof()`는 배열 전체의 byte 크기를 계산하며 문자열 배열에서는 `\0`도 포함한다.

### 한 줄 정리

임베디드 코드에서는 데이터의 실제 크기와 용도를 명확히 하기 위해 `uint8_t`, `uint16_t`, `uint32_t` 같은 고정 폭 정수형을 사용한다.

---

## DS-01에서 확인한 32-bit·64-bit와 구조체 크기

### 구현 목표

`09_DataStructures_Basic`의 `CommandBuffer` 선언을 기준으로 배열 크기, `size_t`, CPU bit 수, 구조체 크기의 관계를 구분한다.

### 핵심 코드

```c
#define COMMAND_BUFFER_CAPACITY 16U

typedef struct
{
    char data[COMMAND_BUFFER_CAPACITY];
    size_t length;
} CommandBuffer;
```

### 32-bit와 64-bit

보통 PC 설정에서 `x86`은 32-bit, `x64`는 64-bit CPU·실행 환경을 뜻한다. `x32`라는 표기는 일반적인 CPU 이름으로는 쓰지 않으므로, 여기서는 32-bit 환경을 `x86` 또는 32-bit라고 부른다.

| 실행 환경 | 주소·포인터 크기 | 이 학습과의 관계 |
|---|---:|---|
| PC x64 (MSVC) | 보통 8byte | `size_t`도 보통 8byte |
| STM32F401RE (32-bit ARM Cortex-M4) | 4byte | `size_t`도 4byte |

`size_t`는 배열의 길이와 메모리 크기를 표현하는 unsigned 자료형이다. 정확한 byte 수는 컴파일 대상에 따라 달라지므로, 고정 폭이 필요한 통신 데이터에는 `uint32_t`처럼 크기가 이름에 있는 자료형을 사용한다.

### `CommandBuffer` 크기 계산

`char`는 1byte이므로 `data[16]`은 항상 16byte다. 구조체 전체에는 `length`도 포함된다.

```text
PC x64의 일반적인 계산
data[16]  : 16byte
size_t    :  8byte
합계      : 24byte

STM32F401RE의 일반적인 계산
data[16]  : 16byte
size_t    :  4byte
합계      : 20byte
```

구조체 멤버의 순서와 CPU 정렬 규칙에 따라 Padding byte가 들어갈 수도 있다. 정확한 전체 크기는 대상 환경에서 `sizeof(CommandBuffer)`로 확인한다. 현재 DS-01은 `sizeof()` 값을 출력하지 않았으므로, 위 값은 자료형 크기에 따른 계산이며 실행 출력으로 검증한 값은 아니다.

### 빌드 상태

- PC x64 MSVC Build Tools에서 CMake 빌드와 CTest 1개 통과
- STM32CubeIDE의 ARM GCC에서 문법·경고 검사 통과
- 이 챕터는 PC C 학습 코드이므로 STM32 보드 다운로드·검증은 하지 않음

### 배운 점

- 배열의 칸 수는 `COMMAND_BUFFER_CAPACITY`가 컴파일 전에 정한다.
- `data` 배열의 크기와 구조체 전체 크기는 다르다.
- 같은 C 코드라도 PC x64와 STM32 32-bit ARM에서 `size_t`·포인터 크기가 달라질 수 있다.

### 한 줄 정리

`CommandBuffer`의 문자 배열은 항상 16byte지만, `size_t`와 정렬 규칙 때문에 전체 구조체 크기는 빌드 대상에 따라 달라진다.
