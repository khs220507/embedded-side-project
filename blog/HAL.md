# STM32 HAL: GPIO와 UART 제어

## HAL이란?

HAL은 `Hardware Abstraction Layer`의 약자로, **하드웨어 추상화 계층**을 의미한다. STM32의 레지스터를 직접 조작하지 않고도 주변장치를 제어할 수 있도록 STMicroelectronics가 제공하는 드라이버 함수 모음이다.

```text
사용자 코드
→ STM32 HAL 함수
→ MCU 레지스터
→ GPIO, UART 등의 실제 하드웨어
```

현재 프로젝트에서는 다음과 같은 HAL 함수를 사용한다.

| 함수 | 역할 |
|---|---|
| `HAL_Init()` | HAL과 1ms SysTick의 기본 초기화 |
| `HAL_GPIO_WritePin()` | GPIO 출력값 변경 |
| `HAL_GPIO_ReadPin()` | GPIO 입력값 읽기 |
| `HAL_GetTick()` | 시스템 시작 후 경과 시간 확인 |
| `HAL_UART_Init()` | UART 설정을 실제 주변장치에 적용 |
| `HAL_UART_Transmit()` | UART로 데이터 송신 |

## HAL을 사용하는 이유

레지스터를 직접 제어하면 MCU 내부 구조를 자세히 이해할 수 있지만, 주변장치마다 주소와 bit 위치를 직접 계산해야 한다. HAL은 이 작업을 함수와 구조체로 감싸 의미 있는 이름으로 하드웨어를 제어하게 해준다.

GPIO 출력을 예로 들면 다음 HAL 코드는 PB5를 `HIGH`로 만든다.

```c
HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
```

HAL 내부에서는 GPIO Port와 Pin을 확인하고 해당 GPIO 레지스터에 값을 기록한다.

```text
HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET)
→ GPIOB의 출력 레지스터 제어
→ PB5가 HIGH로 변경
→ LED 켜짐
```

HAL을 사용하면 다음 장점이 있다.

- 함수 이름으로 코드의 목적을 쉽게 파악할 수 있다.
- 레지스터 주소와 bit 계산을 반복하지 않아도 된다.
- 같은 STM32 HAL 구조를 GPIO, UART, Timer 등에서 일관되게 사용할 수 있다.
- 상태값과 오류 처리 방식이 정해져 있어 동작 결과를 확인하기 쉽다.

HAL을 사용하더라도 Pin 설정, 통신 속도, 데이터 길이와 같은 하드웨어 조건은 개발자가 정확히 지정해야 한다.

## HAL 드라이버의 위치

현재 프로젝트의 HAL 드라이버는 다음 디렉터리에 있다.

```text
01_GPIO_LED/
└── Drivers/
    └── STM32F4xx_HAL_Driver/
        ├── Inc/
        └── Src/
```

- `Inc/`: 함수 원형, 자료형, 상수 정의가 있는 Header 파일
- `Src/`: HAL 함수의 실제 동작 코드가 있는 Source 파일

UART 송신 함수는 다음 두 파일에서 확인할 수 있다.

```text
Inc/stm32f4xx_hal_uart.h
Src/stm32f4xx_hal_uart.c
```

Header 파일에는 함수 원형이 선언되어 있다.

```c
HAL_StatusTypeDef HAL_UART_Transmit(
    UART_HandleTypeDef *huart,
    const uint8_t *pData,
    uint16_t Size,
    uint32_t Timeout
);
```

Source 파일에는 UART 상태 확인, byte 송신, Timeout 처리 등의 실제 구현이 들어 있다.

## Handle 구조체

HAL은 주변장치의 설정과 현재 상태를 `Handle` 구조체로 관리한다.

```c
UART_HandleTypeDef huart2;
```

현재 프로젝트에서 `huart2`에는 USART2를 사용하기 위한 정보가 저장된다.

```c
huart2.Instance = USART2;
huart2.Init.BaudRate = 115200;
huart2.Init.WordLength = UART_WORDLENGTH_8B;
huart2.Init.StopBits = UART_STOPBITS_1;
huart2.Init.Parity = UART_PARITY_NONE;
huart2.Init.Mode = UART_MODE_TX_RX;
```

송신 함수에는 이 구조체의 주소를 전달한다.

```c
HAL_UART_Transmit(&huart2,
                  message,
                  sizeof(message) - 1U,
                  HAL_MAX_DELAY);
```

`&huart2`를 통해 HAL은 USART2의 설정과 현재 동작 상태를 확인하고 갱신할 수 있다.

## 초기화 함수와 동작 함수

HAL 함수는 크게 **초기화 함수**와 **동작 함수**로 나누어 볼 수 있다.

### 초기화 함수

주변장치를 사용하기 전에 동작 조건을 하드웨어에 적용한다.

```c
HAL_Init();
MX_GPIO_Init();
MX_USART2_UART_Init();
```

`MX_GPIO_Init()`과 `MX_USART2_UART_Init()`은 STM32CubeMX가 생성한 프로젝트 초기화 함수다. 함수 내부에서 GPIO 설정 구조체와 UART Handle을 준비한 뒤 `HAL_GPIO_Init()`, `HAL_UART_Init()` 같은 HAL 함수를 호출한다.

### 동작 함수

초기화가 끝난 주변장치로 실제 입출력을 수행한다.

```c
HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin);
HAL_GPIO_WritePin(GPIOB, GREEN_LED_Pin, GPIO_PIN_SET);
HAL_UART_Transmit(&huart2, message, sizeof(message) - 1U,
                  HAL_MAX_DELAY);
```

따라서 UART 송신은 반드시 USART2 초기화 이후에 실행되어야 한다.

```text
HAL_Init()
→ MX_GPIO_Init()
→ MX_USART2_UART_Init()
→ HAL_UART_Transmit()
```

## HAL 함수의 반환값

여러 HAL 함수는 HAL 드라이버에 미리 정의된 `HAL_StatusTypeDef`로 실행 결과를 반환한다. `HAL_OK` 역시 사용자가 만든 값이 아니라 HAL에서 제공하는 상태값이다.

```c
typedef enum
{
    HAL_OK      = 0x00U,
    HAL_ERROR   = 0x01U,
    HAL_BUSY    = 0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;
```

| 반환값 | 의미 |
|---|---|
| `HAL_OK` | 정상 완료 |
| `HAL_ERROR` | 동작 중 오류 발생 |
| `HAL_BUSY` | 주변장치가 다른 작업을 수행 중 |
| `HAL_TIMEOUT` | 제한 시간 안에 완료되지 않음 |

현재 UART 수신 코드에서는 `HAL_UART_Receive()`의 반환값을 바로 검사한다.

```c
if (HAL_UART_Receive(&huart2, &receivedByte, 1U, 0U) == HAL_OK)
{
    UART_HandleCommand(receivedByte);
}
```

문자 1바이트를 정상적으로 받으면 `HAL_OK`가 반환되어 명령 처리 함수를 호출한다. Timeout이 `0U`이므로 수신 데이터가 없을 때는 기다리지 않고 `HAL_TIMEOUT`으로 돌아가며 조건문 내부를 실행하지 않는다.

## HAL과 직접 레지스터 제어의 관계

HAL과 레지스터 제어는 서로 다른 하드웨어를 사용하는 방식이 아니다. HAL 함수도 최종적으로는 같은 STM32 레지스터를 읽고 쓴다.

```text
HAL 사용      : 함수 호출 → HAL 내부 코드 → 레지스터
직접 제어     : 사용자 코드 → 레지스터
```

현재 단계에서는 HAL로 GPIO와 UART의 동작 흐름을 먼저 익히고, 레지스터 직접 제어는 하드웨어 내부 구조를 더 깊게 학습할 때 다루는 것이 적절하다.

## 배운 점

- HAL은 STM32 주변장치를 함수와 구조체로 제어하게 해주는 하드웨어 추상화 계층이다.
- HAL 함수 내부에서도 최종적으로 MCU 레지스터를 읽거나 쓴다.
- 주변장치 설정과 상태는 `Handle` 구조체에 저장한다.
- 주변장치를 초기화한 다음 HAL 동작 함수를 호출해야 한다.
- 여러 HAL 함수는 `HAL_OK`, `HAL_ERROR`, `HAL_BUSY`, `HAL_TIMEOUT`으로 결과를 반환한다.

## 한 줄 정리

STM32 HAL은 레지스터 제어를 함수와 구조체로 감싸 GPIO와 UART 같은 하드웨어를 일관된 방식으로 사용할 수 있게 해주는 드라이버 계층이다.
