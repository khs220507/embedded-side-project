# STM32 DMA: UART 송수신과 ReceiveToIdle

## 기본 개념

DMA(Direct Memory Access)는 CPU 대신 Peripheral과 Memory 사이의 데이터를 옮기는 하드웨어다. CPU는 전송 방향, Buffer 주소와 크기를 설정하고, DMA는 실제 데이터 이동을 수행한 뒤 Interrupt로 완료를 알린다.

```text
CPU: DMA 설정과 전송 시작
→ DMA: Peripheral과 Memory 사이의 데이터 이동
→ CPU: 완료 IRQ와 Callback 처리
```

STM32F4의 DMA 요청은 Controller, Stream과 Channel 조합으로 Peripheral에 연결한다.

| 용어 | 의미 |
|---|---|
| DMA Controller | 여러 DMA Stream을 관리하는 장치 |
| Stream | 실제 전송을 수행하는 통로 |
| Channel | Stream에 Peripheral 요청을 연결하는 선택 값 |
| Direction | Peripheral과 Memory 사이의 전송 방향 |
| Normal Mode | 한 번의 전송이 끝나면 정지하는 Mode |

## 구현 목표

`03_UART_DMA`에서는 `02_UART`의 문자열 LED 명령을 유지하면서 USART2 RX와 TX의 데이터 이동을 DMA1에 맡긴다.

| 방향 | DMA 자원 | 데이터 이동 |
|---|---|---|
| RX | DMA1 Stream5, Channel 4 | USART2 Data Register → RX Buffer |
| TX | DMA1 Stream6, Channel 4 | TX Buffer → USART2 Data Register |

## 핵심 코드

### RX/TX DMA 설정

RX는 Peripheral에서 Memory로, TX는 Memory에서 Peripheral로 이동한다. USART2 Data Register 주소는 고정하고 배열의 Memory 주소만 증가시킨다.

```c
hdma_usart2_rx.Instance = DMA1_Stream5;
hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
hdma_usart2_rx.Init.Mode = DMA_NORMAL;
```

```c
hdma_usart2_tx.Instance = DMA1_Stream6;
hdma_usart2_tx.Init.Channel = DMA_CHANNEL_4;
hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
hdma_usart2_tx.Init.Mode = DMA_NORMAL;
```

### UART와 DMA Handle 연결

```c
__HAL_LINKDMA(huart, hdmarx, hdma_usart2_rx);
__HAL_LINKDMA(huart, hdmatx, hdma_usart2_tx);
```

UART Handle의 `hdmarx`, `hdmatx`와 DMA Handle의 `Parent`가 연결되어 UART DMA 함수와 IRQ Handler가 같은 상태를 공유한다.

### DMA Clock과 NVIC

```c
__HAL_RCC_DMA1_CLK_ENABLE();

HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
```

DMA Register를 사용하기 전에 DMA1 Clock을 활성화하고 RX/TX Stream IRQ를 NVIC에서 허용한다.

### ReceiveToIdle DMA 수신

길이가 일정하지 않은 문자열은 Buffer 전체가 찰 때까지 기다리지 않도록 `ReceiveToIdle DMA`로 수신한다.

```c
static void UART_StartDmaReceive(void)
{
    if (HAL_UARTEx_ReceiveToIdle_DMA(&huart2,
                                     uartDmaRxBuffer,
                                     UART_RX_BUFFER_SIZE) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
}
```

IDLE 또는 Transfer Complete Event가 발생하면 실제 수신 크기를 처리하고 다음 DMA 수신을 등록한다.

```c
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart,
                               uint16_t size)
{
    if (huart->Instance == USART2)
    {
        UART_StoreReceivedData(uartDmaRxBuffer, size);
        UART_StartDmaReceive();
    }
}
```

### DMA Buffer와 명령 Buffer

```c
static uint8_t uartDmaRxBuffer[UART_RX_BUFFER_SIZE];
static char uartRxBuffer[UART_RX_BUFFER_SIZE];
```

- `uartDmaRxBuffer`: DMA가 직접 기록하는 임시 Buffer
- `uartRxBuffer`: 여러 DMA Event의 문자를 Enter까지 누적하는 C 문자열 Buffer

DMA가 여러 바이트를 전달하면 `UART_StoreReceivedData()`가 문자별로 Enter, Backspace와 Overflow를 처리한다.

### DMA 송신

`printf()`가 호출하는 `_write()`에서는 문자열을 고정 TX Buffer에 복사하고 DMA 송신을 시작한다.

```c
memcpy(uartDmaTxBuffer, &ptr[sent], chunkSize);
uartDmaTxComplete = 0U;

if (HAL_UART_Transmit_DMA(&huart2,
                          uartDmaTxBuffer,
                          chunkSize) != HAL_OK)
{
    return -1;
}

while (uartDmaTxComplete == 0U)
{
    __WFI();
}
```

DMA 송신이 끝나면 Callback이 완료 Flag를 변경한다.

```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        uartDmaTxComplete = 1U;
    }
}
```

현재 `_write()`는 DMA로 데이터를 옮기지만 완료 Callback까지 기다리므로 호출 방식은 동기식이다. `__WFI()`의 CPU 동작은 [`CMSIS.md`](../fundamentals/CMSIS.md)에서 관리한다.

## 코드 설명

### RX와 TX 주소 이동

```text
RX: 고정된 USART2 Data Register → 증가하는 RX Buffer 주소
TX: 증가하는 TX Buffer 주소 → 고정된 USART2 Data Register
```

UART는 한 바이트 Data Register를 계속 사용하므로 Peripheral 주소는 증가하지 않는다. 반면 배열에는 다음 위치로 이동하며 저장하거나 읽어야 하므로 Memory 주소는 증가한다.

### Normal Mode 재등록

현재 RX와 TX는 `DMA_NORMAL`이다. 한 번의 전송이 끝나면 Stream이 정지하므로 RX Event Callback에서 `UART_StartDmaReceive()`를 다시 호출한다.

### DMA IRQ와 UART IRQ

DMA Stream IRQ는 Transfer Complete와 DMA Error를 처리하고, USART2 IRQ는 IDLE 감지와 UART의 최종 Transmission Complete 등을 처리한다.

```text
Hardware Event
→ DMA Stream IRQ 또는 USART2 IRQ
→ HAL IRQ Handler
→ UART DMA Callback
```

## Interrupt 방식과 DMA 방식 비교

| 항목 | Interrupt 수신 | DMA 수신 |
|---|---|---|
| 데이터 이동 | CPU가 한 바이트씩 처리 | DMA가 여러 바이트를 Buffer로 이동 |
| Callback 입력 | `receivedByte` 한 바이트 | Buffer와 실제 수신 크기 |
| 완료 처리 | 바이트마다 Callback | IDLE 또는 Buffer 완료 Event |
| 다음 수신 | 한 바이트씩 재등록 | Normal Mode 수신 재등록 |
| 학습 프로젝트 | `02_UART` | `03_UART_DMA` |

## 동작 흐름

```text
RX DMA 수신 등록
→ DMA1 Stream5가 USART2 데이터를 임시 Buffer에 저장
→ UART IDLE 또는 DMA Transfer Complete
→ HAL_UARTEx_RxEventCallback()
→ 문자열 명령 Buffer에 누적
→ Enter에서 명령 완성
→ Main Loop에서 LED 명령 처리
→ DMA1 Stream6으로 처리 결과 송신
→ HAL_UART_TxCpltCallback()에서 완료 Flag 설정
```

## 실행 결과

- `02_UART`와 `03_UART_DMA`의 Debug 빌드 및 ELF 생성을 확인했다.
- STM32CubeProgrammer에서 `03_UART_DMA.elf`의 Flash 기록과 Verify를 완료했다.
- 실제 NUCLEO-F401RE에서 UART DMA 송수신과 문자열 LED 명령 동작을 확인했다.

## 배운 점

- DMA는 CPU 대신 Peripheral과 Memory 사이의 데이터를 이동한다.
- STM32F4에서는 Peripheral 요청에 맞는 Stream과 Channel 조합을 선택해야 한다.
- DMA를 사용하려면 Clock, NVIC, IRQ Handler와 Callback이 필요하다.
- `ReceiveToIdle DMA`는 길이가 일정하지 않은 UART 데이터 수신에 사용할 수 있다.
- DMA Buffer와 애플리케이션 문자열 Buffer는 목적에 따라 분리할 수 있다.
- DMA 함수 자체가 Non-blocking이어도 호출 코드가 완료를 기다리면 전체 동작은 동기식이 될 수 있다.

## 한 줄 정리

> USART2와 Memory 사이의 데이터 이동을 DMA1에 맡기고, CPU는 수신이 끝난 문자열 명령과 완료 Event를 처리하도록 구현했다.
