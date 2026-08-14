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
