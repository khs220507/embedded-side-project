# STM32 HAL: GPIO, UART, DMA, ADC와 Timer 제어

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
| `HAL_UART_Receive_IT()` | UART Interrupt 수신 시작 |
| `HAL_DMA_Init()` | DMA Handle 설정을 DMA 주변장치에 적용 |
| `HAL_UART_Transmit_DMA()` | DMA 방식 UART 송신 시작 |
| `HAL_UARTEx_ReceiveToIdle_DMA()` | IDLE 감지를 포함한 가변 길이 DMA 수신 시작 |
| `HAL_NVIC_SetPriority()` | Peripheral IRQ 우선순위 설정 |
| `HAL_NVIC_EnableIRQ()` | NVIC에서 Peripheral IRQ 활성화 |
| `HAL_UART_IRQHandler()` | UART IRQ 원인을 확인하고 HAL의 UART 처리로 전달 |
| `HAL_ADC_Init()` | ADC Handle 설정을 ADC1에 적용 |
| `HAL_ADC_ConfigChannel()` | 변환할 ADC Channel, Rank와 Sampling Time 설정 |
| `HAL_ADC_Start()` | Software Trigger 방식의 ADC 변환 시작 |
| `HAL_ADC_PollForConversion()` | ADC 변환 완료를 제한 시간 동안 대기 |
| `HAL_ADC_GetValue()` | 완료된 ADC 변환값 읽기 |
| `HAL_ADC_Stop()` | ADC 변환 정지 |
| `__HAL_RCC_ADC1_CLK_ENABLE()` | ADC1 Peripheral Clock 활성화 |
| `HAL_TIM_Base_Init()` | Timer Base 설정을 TIM2에 적용 |
| `HAL_TIM_Base_Start_IT()` | Timer Counter와 Update Interrupt 시작 |
| `HAL_TIM_IRQHandler()` | TIM2 IRQ 원인을 HAL Timer 처리로 전달 |
| `HAL_TIM_PeriodElapsedCallback()` | Update Event 처리 뒤 호출되는 사용자 Callback |
| `__HAL_RCC_TIM2_CLK_ENABLE()` | TIM2 Peripheral Clock 활성화 |

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

UART Interrupt 수신을 시작할 때도 반환값을 검사한다.

```c
if (HAL_UART_Receive_IT(&huart2, &receivedByte, 1U) != HAL_OK)
{
    Error_Handler();
}
```

`HAL_OK`이면 수신이 정상적으로 등록된 것이고 함수는 즉시 반환한다. 이후 문자가 도착하면 USART2 IRQ와 `HAL_UART_RxCpltCallback()`을 통해 수신 완료를 알린다.

## UART DMA에서 사용하는 HAL

`03_UART_DMA`에서는 `DMA_HandleTypeDef` 두 개로 USART2 RX와 TX의 DMA 설정과 상태를 관리한다.

```c
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;
```

### `__HAL_RCC_DMA1_CLK_ENABLE()`

DMA1에 Peripheral Clock을 공급하는 HAL Macro다.

```c
__HAL_RCC_DMA1_CLK_ENABLE();
```

STM32는 사용하지 않는 주변장치의 Clock을 꺼 전력 소모를 줄인다. DMA Register를 설정하거나 DMA 전송을 시작하기 전에 DMA1 Clock을 먼저 활성화해야 한다.

### `HAL_NVIC_SetPriority()`와 `HAL_NVIC_EnableIRQ()`

DMA Stream에서 발생하는 Interrupt의 우선순위를 정하고 NVIC에서 해당 IRQ를 활성화한다.

```c
HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
```

| 함수 | 핵심 인자 | 역할 |
|---|---|---|
| `HAL_NVIC_SetPriority()` | IRQ, Preemption Priority, Subpriority | IRQ 우선순위 설정 |
| `HAL_NVIC_EnableIRQ()` | IRQ | NVIC에서 해당 Interrupt 허용 |

현재 설정에서 Stream5는 USART2 RX DMA, Stream6은 USART2 TX DMA가 사용한다. 두 IRQ 모두 Preemption Priority와 Subpriority를 `0`으로 설정했다.

`MX_DMA_Init()`은 STM32CubeMX가 만든 Project 초기화 함수이며 HAL 함수가 아니다. 이 함수 내부에서 DMA Clock과 NVIC 설정을 위해 위 HAL Macro와 함수를 호출한다.

### `HAL_DMA_Init()`

DMA Handle에 지정한 Stream, Channel, 전송 방향, 주소 증가 방식과 데이터 단위를 실제 DMA Register에 적용한다.

```c
if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
{
    Error_Handler();
}
```

| 인자 | 의미 |
|---|---|
| `&hdma_usart2_rx` | 초기화할 RX DMA Handle의 주소 |
| 반환값 | `HAL_OK`, `HAL_ERROR` 등의 `HAL_StatusTypeDef` |

### `__HAL_LINKDMA()`

`__HAL_LINKDMA()`는 UART Handle과 DMA Handle을 연결하는 HAL Macro다.

```c
__HAL_LINKDMA(huart, hdmarx, hdma_usart2_rx);
__HAL_LINKDMA(huart, hdmatx, hdma_usart2_tx);
```

| 인자 | RX 예시 | 의미 |
|---|---|---|
| Peripheral Handle | `huart` | DMA를 사용할 UART Handle |
| DMA Field | `hdmarx` | UART Handle 내부의 RX DMA 연결 필드 |
| DMA Handle | `hdma_usart2_rx` | 연결할 DMA Handle |

Macro 실행 후 UART Handle의 `hdmarx` 또는 `hdmatx`가 DMA Handle을 가리키고, DMA Handle의 `Parent`가 UART Handle을 가리킨다. 이 연결 덕분에 UART DMA 함수와 DMA IRQ Handler가 같은 전송 상태를 공유한다.

```text
huart2.hdmarx → hdma_usart2_rx
hdma_usart2_rx.Parent → huart2
```

### `HAL_UART_Transmit_DMA()`

Memory Buffer의 데이터를 DMA로 UART에 송신하도록 시작한다.

```c
HAL_UART_Transmit_DMA(&huart2, uartDmaTxBuffer, chunkSize);
```

| 인자 | 의미 |
|---|---|
| `&huart2` | 송신에 사용할 UART Handle |
| `uartDmaTxBuffer` | DMA가 읽을 Memory Buffer |
| `chunkSize` | 송신할 바이트 수 |

함수는 송신 완료를 기다리지 않고 전송을 시작한 뒤 반환한다. DMA와 UART 송신이 끝나면 `HAL_UART_TxCpltCallback()`이 호출된다. 현재 `_write()`는 이 Callback이 Flag를 바꿀 때까지 `__WFI()`로 기다리므로 호출 방식은 동기식이다.

`__WFI()`는 HAL이 아니라 CMSIS-Core가 제공하는 CPU Intrinsic이므로 자세한 내용은 [`CMSIS.md`](CMSIS.md)에서 관리한다.

### `HAL_UARTEx_ReceiveToIdle_DMA()`

정해진 Buffer 크기만큼 DMA 수신을 시작하면서 UART IDLE 상태도 감지한다.

```c
HAL_UARTEx_ReceiveToIdle_DMA(&huart2,
                             uartDmaRxBuffer,
                             UART_RX_BUFFER_SIZE);
```

| 인자 | 의미 |
|---|---|
| `&huart2` | 수신에 사용할 UART Handle |
| `uartDmaRxBuffer` | DMA가 데이터를 저장할 Memory Buffer |
| `UART_RX_BUFFER_SIZE` | 수신 가능한 최대 바이트 수 |

Buffer가 가득 차거나 RX 선이 IDLE 상태가 되면 실제 수신 크기와 함께 `HAL_UARTEx_RxEventCallback()`이 호출된다. 현재 RX DMA는 `DMA_NORMAL`이므로 Callback에서 다음 수신을 다시 등록한다.

### `__HAL_DMA_DISABLE_IT()`

특정 DMA Interrupt를 비활성화하는 HAL Macro다.

```c
__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
```

현재 코드는 Buffer가 절반 찼을 때 발생하는 Half Transfer Interrupt인 `DMA_IT_HT`를 사용하지 않으므로 비활성화한다. IDLE 또는 Transfer Complete 이벤트만 처리하면 같은 수신 데이터를 중간 단계에서 불필요하게 처리하지 않아도 된다.

### `HAL_DMA_IRQHandler()`

DMA Stream IRQ에서 호출해 발생한 DMA Flag를 확인하고 HAL의 완료 또는 오류 처리로 전달한다.

```c
void DMA1_Stream5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_usart2_rx);
}
```

RX는 DMA1 Stream5, TX는 DMA1 Stream6 IRQ에서 각각 자신의 DMA Handle을 전달한다.

### `HAL_UART_IRQHandler()`

USART IRQ에서 호출해 RXNE, IDLE, TC, 오류 등의 UART Interrupt 원인을 확인하고 HAL의 해당 처리로 전달한다.

```c
void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}
```

현재 DMA 수신에서는 IDLE 감지, DMA 송신에서는 UART의 최종 Transmission Complete 처리에 사용된다. DMA Stream IRQ와 USART2 IRQ는 서로 대체하는 관계가 아니며, 전송 단계에 따라 둘 다 사용될 수 있다.

### DMA Callback

Callback은 애플리케이션 코드에서 구현하지만 이름과 호출 시점은 HAL이 정의한다.

```c
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart,
                               uint16_t size);

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
```

- `HAL_UARTEx_RxEventCallback()`: IDLE 또는 DMA 수신 완료 시 실제 수신 크기를 전달한다.
- `HAL_UART_TxCpltCallback()`: DMA 기반 UART 송신이 끝났음을 알린다.

```text
UART DMA 함수 호출
→ DMA가 데이터 이동
→ DMA Stream IRQ 또는 USART2 IRQ
→ HAL_DMA_IRQHandler() 또는 HAL_UART_IRQHandler()
→ UART 상태 처리
→ UART DMA Callback
```

DMA 코드의 Debug 빌드와 ELF 생성, Flash 기록과 Verify를 완료했다. 실제 보드에서 UART DMA 송수신과 문자열 LED 명령 동작도 확인했다.

## ADC에서 사용하는 HAL

`04_ADC_CdS`에서는 `ADC_HandleTypeDef hadc1`에 ADC1의 설정과 상태를 저장한다. PA0는 `ADC1_IN0` Analog Input으로 설정했으며, 12-bit 단일 변환을 Software Trigger로 실행한다.

### `HAL_ADC_Init()`

ADC Clock Prescaler, Resolution, Scan/Continuous Mode, 정렬 방식과 변환 개수 등의 Handle 설정을 실제 ADC1에 적용한다.

```c
if (HAL_ADC_Init(&hadc1) != HAL_OK)
{
    Error_Handler();
}
```

- 인자: 초기화할 `ADC_HandleTypeDef`의 주소
- 반환값: `HAL_OK`, `HAL_ERROR`, `HAL_BUSY`, `HAL_TIMEOUT`
- 연결 관계: 내부에서 `HAL_ADC_MspInit()`을 호출해 ADC1 Clock과 PA0 Analog Mode를 준비한다.

`HAL_ADC_MspInit()`에서는 `__HAL_RCC_ADC1_CLK_ENABLE()` Macro로 ADC1 Peripheral Clock을 활성화한다. Clock이 공급되지 않으면 ADC Register 설정과 변환이 동작하지 않는다. 현재 ADC는 Interrupt를 활성화하지 않으므로 ADC IRQ나 변환 완료 Callback과 연결되지 않는다.

### `HAL_ADC_ConfigChannel()`

Regular Conversion에 사용할 Channel, Rank와 Sampling Time을 설정한다.

```c
channelConfig.Channel = ADC_CHANNEL_0;
channelConfig.Rank = 1;
channelConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;
HAL_ADC_ConfigChannel(&hadc1, &channelConfig);
```

현재 코드는 변환 순서가 하나뿐이므로 PA0에 연결된 `ADC_CHANNEL_0`을 Rank 1로 사용한다.

### `HAL_ADC_Start()` → `HAL_ADC_PollForConversion()` → `HAL_ADC_GetValue()` → `HAL_ADC_Stop()`

```c
HAL_ADC_Start(&hadc1);
HAL_ADC_PollForConversion(&hadc1, 10U);
rawValue = HAL_ADC_GetValue(&hadc1);
HAL_ADC_Stop(&hadc1);
```

| 함수 | 핵심 인자와 반환값 | 현재 역할 |
|---|---|---|
| `HAL_ADC_Start()` | `&hadc1`, `HAL_StatusTypeDef` | Software Trigger 변환 시작 |
| `HAL_ADC_PollForConversion()` | `&hadc1`, Timeout 10ms | EOC Flag가 설정될 때까지 Polling |
| `HAL_ADC_GetValue()` | `&hadc1`, `uint32_t` 변환값 | 12-bit 결과인 0~4095 읽기 |
| `HAL_ADC_Stop()` | `&hadc1`, `HAL_StatusTypeDef` | 이번 단일 변환 종료 |

현재 ADC 경로는 Interrupt나 Callback을 사용하지 않는다. CPU가 최대 10ms 동안 완료를 확인하는 Polling 방식이며, 실제 변환은 보통 Timeout보다 훨씬 빨리 끝난다.

## Timer에서 사용하는 HAL

`05_Timer_Sampling`에서는 `TIM_HandleTypeDef htim2`가 TIM2의 설정과 상태를 저장한다. TIM2는 84MHz APB1 Timer Clock을 Prescaler `8399`, Period `4999`로 나누어 500ms마다 Update Event를 만든다.

### `HAL_TIM_Base_Init()`과 `HAL_TIM_Base_Start_IT()`

```c
HAL_TIM_Base_Init(&htim2);
HAL_TIM_Base_Start_IT(&htim2);
```

`HAL_TIM_Base_Init()`은 Counter Mode, Prescaler, Period 설정을 적용하고 내부에서 `HAL_TIM_Base_MspInit()`을 호출한다. 현재 MSP Init은 `__HAL_RCC_TIM2_CLK_ENABLE()`로 TIM2 Clock을 공급하고 `HAL_NVIC_SetPriority()`·`HAL_NVIC_EnableIRQ()`로 `TIM2_IRQn`을 활성화한다.

`HAL_TIM_Base_Start_IT()`은 TIM2 Counter를 시작하고 Update Interrupt를 허용한다. 인자는 모두 `&htim2`이며, 두 함수 모두 성공 시 `HAL_OK`를 반환한다.

### `HAL_TIM_IRQHandler()`와 `HAL_TIM_PeriodElapsedCallback()`

```c
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        timerSampleRequested = 1U;
    }
}
```

`TIM2_IRQHandler()`에서 `HAL_TIM_IRQHandler()`가 Update Flag를 처리하면 HAL이 `HAL_TIM_PeriodElapsedCallback()`을 호출한다. Callback은 HAL이 제공하는 약한(weak) 함수이므로 사용자 코드에서 같은 이름으로 구현해 동작을 연결한다. 이 프로젝트는 Interrupt에서 Flag만 설정하고 ADC Polling·LED·UART 출력은 Main Loop가 수행한다.

## HAL과 직접 레지스터 제어의 관계

HAL과 레지스터 제어는 서로 다른 하드웨어를 사용하는 방식이 아니다. HAL 함수도 최종적으로는 같은 STM32 레지스터를 읽고 쓴다.

```text
HAL 사용      : 함수 호출 → HAL 내부 코드 → 레지스터
직접 제어     : 사용자 코드 → 레지스터
```

현재 단계에서는 HAL로 GPIO, UART와 ADC의 동작 흐름을 먼저 익히고, 레지스터 직접 제어는 하드웨어 내부 구조를 더 깊게 학습할 때 다루는 것이 적절하다.

## 배운 점

- HAL은 STM32 주변장치를 함수와 구조체로 제어하게 해주는 하드웨어 추상화 계층이다.
- HAL 함수 내부에서도 최종적으로 MCU 레지스터를 읽거나 쓴다.
- 주변장치 설정과 상태는 `Handle` 구조체에 저장한다.
- 주변장치를 초기화한 다음 HAL 동작 함수를 호출해야 한다.
- 여러 HAL 함수는 `HAL_OK`, `HAL_ERROR`, `HAL_BUSY`, `HAL_TIMEOUT`으로 결과를 반환한다.
- `__HAL_LINKDMA()`는 UART Handle과 DMA Handle을 양방향으로 연결한다.
- UART DMA 함수는 전송을 시작한 뒤 IRQ와 Callback으로 완료를 알린다.
- ADC Polling 변환은 시작, 완료 대기, 결과 읽기, 정지 순서로 수행한다.
- Timer Update Interrupt는 IRQ Handler → HAL IRQ Handler → Callback 순서로 전달된다.

## 한 줄 정리

STM32 HAL은 레지스터 제어를 함수, Macro와 Handle 구조체로 감싸 GPIO, UART, DMA, ADC와 Timer를 일관된 방식으로 사용할 수 있게 해주는 드라이버 계층이다.

## PWM에서 사용하는 HAL

`06_PWM_LED`에서는 `TIM_HandleTypeDef htim1`이 TIM1 PWM의 설정과 상태를 저장한다. `HAL_TIM_PWM_Init()`은 TIM1의 PWM 동작을 초기화하고 `HAL_TIM_PWM_MspInit()`을 통해 TIM1 Clock을 활성화한다. 성공하면 `HAL_OK`를 반환한다.

`HAL_TIM_PWM_ConfigChannel(&htim1, &channelConfig, TIM_CHANNEL_1)`은 Channel 1의 PWM Mode, Polarity, 초기 Compare 값을 설정한다. 이 프로젝트는 `TIM_OCMODE_PWM1`, Pulse `0`을 사용한다.

`HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1)`은 설정된 PWM 출력을 시작한다. PA8은 `HAL_TIM_MspPostInit()`에서 `GPIO_AF1_TIM1` Alternate Function으로 설정되어 `TIM1_CH1` 신호를 출력한다.

`__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse)`는 HAL Macro다. TIM1을 멈추지 않고 Channel 1의 Compare Register를 `pulse` 값으로 갱신한다. 현재 코드는 CdS ADC 원시값 `0~4095`를 `0~1000` 범위로 변환해 이 Macro에 전달한다. PWM 자체는 Interrupt나 Callback을 사용하지 않으며, TIM2 Update Interrupt 뒤 Main Loop에서 Compare 값을 바꾼다.
