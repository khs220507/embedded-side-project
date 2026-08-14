# STM32 GPIO 학습 기록

## GPIO 출력으로 LED 점멸하기

## 구현 목표

NUCLEO-F401RE의 `D4(PB5)`에 연결한 초록색 LED를 1초 간격으로 켜고 끈다.

## 핵심 코드

```c
while (1)
{
    HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin);
    HAL_Delay(1000);
}
```

## 코드 설명

`GREEN_LED_GPIO_Port`와 `GREEN_LED_Pin`은 각각 `GPIOB`와 `GPIO_PIN_5`를 의미한다. 따라서 다음 함수는 PB5의 현재 출력 상태를 반전한다.

```c
HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin);
```

- 현재 출력이 `LOW`이면 `HIGH`로 변경한다.
- 현재 출력이 `HIGH`이면 `LOW`로 변경한다.

이 회로에서는 PB5가 `HIGH`일 때 LED가 켜지고, `LOW`일 때 LED가 꺼진다.

```c
HAL_Delay(1000);
```

`HAL_Delay()`의 단위는 밀리초이므로 `1000`은 1초를 의미한다. LED의 상태를 반전한 뒤 1초간 기다리고, `while (1)`에 의해 이 동작을 계속 반복한다.

결과적으로 LED는 다음과 같이 동작한다.

```text
켜짐 → 1초 대기 → 꺼짐 → 1초 대기 → 반복
```

LED의 상태는 1초마다 바뀌며, 켜졌다가 다시 켜질 때까지의 전체 점멸 주기는 2초다.

## 동작에 필요한 GPIO 초기화

반복문을 실행하기 전에 다음 함수가 호출된다.

```c
MX_GPIO_Init();
```

이 함수는 GPIOB의 클록을 활성화하고, PB5를 Push-Pull 출력 핀으로 설정한다. 또한 시작 시 LED가 꺼져 있도록 출력값을 `GPIO_PIN_RESET`으로 초기화한다.

## 이번 구현에서 배운 점

- STM32 펌웨어는 하드웨어를 초기화한 후 `while (1)`에서 원하는 동작을 반복한다.
- `PB5`는 GPIOB 포트의 5번 핀을 뜻한다.
- GPIO 핀은 사용하기 전에 출력 모드로 초기화해야 한다.
- `HAL_GPIO_TogglePin()`은 현재 GPIO 출력 상태를 반전한다.
- `HAL_Delay()`를 사용하면 밀리초 단위의 간단한 점멸 간격을 만들 수 있다.

## 한 줄 정리

PB5를 GPIO 출력으로 설정하고, 무한 반복문에서 출력 상태를 반전한 뒤 1초간 기다리도록 구현하여 초록색 LED를 점멸시켰다.

---

## 여러 GPIO 출력으로 LED 순차 점등하기

### 구현 목표

초록색 `PB5`, 파란색 `PB4`, 노란색 `PB10`, 빨간색 `PA8` LED를 1초 간격으로 하나씩 순차 점등한다. 단일 LED 점멸에서 사용한 GPIO 출력 개념은 반복하지 않고, 여러 핀을 다루면서 생긴 확장 내용만 기록한다.

### 핵심 코드

```c
while (1)
{
    LED_On(GREEN_LED_GPIO_Port, GREEN_LED_Pin);
    HAL_Delay(1000);

    LED_On(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
    HAL_Delay(1000);

    LED_On(YELLOW_LED_GPIO_Port, YELLOW_LED_Pin);
    HAL_Delay(1000);

    LED_On(RED_LED_GPIO_Port, RED_LED_Pin);
    HAL_Delay(1000);
}
```

```c
static void LED_AllOff(void)
{
    HAL_GPIO_WritePin(GPIOA, RED_LED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB,
                      YELLOW_LED_Pin | BLUE_LED_Pin | GREEN_LED_Pin,
                      GPIO_PIN_RESET);
}

static void LED_On(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    LED_AllOff();
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}
```

### 핵심 변화

`LED_On()`은 모든 LED를 먼저 끈 다음 전달받은 GPIO 하나만 켠다. 이 방식으로 이전 LED를 끄는 코드를 매 단계마다 반복하지 않으면서, 항상 LED 하나만 켜진 상태를 유지한다.

GPIOA에 연결된 빨간색 LED와 GPIOB에 연결된 나머지 LED는 포트가 다르므로 `LED_AllOff()`에서 나누어 처리했다. 같은 포트의 여러 핀은 비트 OR 연산자 `|`로 묶어 한 번에 제어할 수 있다.

### 배운 점

- 포트와 핀을 함수 인자로 전달하면 동일한 제어 코드를 여러 LED에 재사용할 수 있다.
- 서로 다른 GPIO 포트의 핀은 각각 별도로 제어해야 한다.
- 같은 포트의 여러 핀은 `|`로 묶어 동시에 `RESET`할 수 있다.
- 공통 동작을 함수로 분리하면 반복 코드와 상태 제어 실수를 줄일 수 있다.

### 한 줄 정리

모든 LED를 끄는 공통 함수와 원하는 LED 하나만 켜는 함수를 분리하여, 네 개의 GPIO 출력을 중복 없이 순차 제어했다.

---

## GPIO 버튼 입력과 디바운싱

PC13의 사용자 버튼을 `GPIO_MODE_INPUT`으로 설정하고 `HAL_GPIO_ReadPin()`으로 반복해서 읽는다. 버튼 접점이 눌리는 순간 짧게 흔들리는 바운스 때문에 한 번의 입력이 여러 번 인식될 수 있어, 입력이 바뀐 뒤 50ms 동안 같은 값이 유지될 때만 확정 상태로 받아들인다.

```c
#define USER_BUTTON_Pin GPIO_PIN_13
#define USER_BUTTON_GPIO_Port GPIOC
#define BUTTON_DEBOUNCE_MS 50U
```

### 입력 처리에 사용하는 값

| 값 | 역할 |
|---|---|
| `buttonReading` | 이번 반복에서 읽은 PC13의 입력값 |
| `lastButtonReading` | 직전 반복에서 읽은 입력값 |
| `stableButtonState` | 50ms 검사를 통과한 확정 입력값 |
| `lastDebounceTime` | 입력이 마지막으로 변한 시각 |

현재 입력은 다음과 같이 읽는다.

```c
GPIO_PinState buttonReading =
    HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin);
```

이 보드에서 버튼을 놓으면 `GPIO_PIN_RESET`, 누르면 `GPIO_PIN_SET`으로 읽힌다.

### 디바운싱 흐름

```c
if (buttonReading != lastButtonReading)
{
    lastDebounceTime = HAL_GetTick();
}

if ((HAL_GetTick() - lastDebounceTime) >= BUTTON_DEBOUNCE_MS)
{
    if (buttonReading != stableButtonState)
    {
        stableButtonState = buttonReading;

        if (stableButtonState == GPIO_PIN_SET)
        {
            /* 버튼 눌림을 한 번 처리 */
        }
    }
}
```

입력값이 직전 값과 달라지면 `HAL_GetTick()`으로 변화 시각을 다시 기록한다. 그 후 같은 입력이 50ms 이상 유지되면 확정 상태를 갱신한다. `HAL_Delay(50)`처럼 프로그램을 멈추지 않고 경과 시간만 비교하는 비차단 방식이다.

안정된 버튼 상태가 `GPIO_PIN_SET`으로 바뀌는 순간에만 상태를 전환한다. 버튼을 계속 누르는 동안에는 `buttonReading`과 `stableButtonState`가 모두 `SET`이므로 다시 처리되지 않는다. 버튼을 놓아 `RESET`이 확정된 뒤 다시 눌러야 다음 입력이 발생한다.

### 전체 동작 흐름

```text
PC13 입력 읽기
→ 직전 입력과 비교
→ 입력이 바뀌면 변화 시각 기록
→ 50ms 이상 유지됐는지 확인
→ 확정 상태 갱신
→ SET으로 바뀐 순간만 버튼 눌림 처리
```

### 한 줄 정리

버튼 입력이 50ms 동안 유지됐는지 확인하고 눌림 순간만 처리하여 바운스와 장시간 눌림에 의한 중복 입력을 방지했다.
