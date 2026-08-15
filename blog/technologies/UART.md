# STM32 UART: 송수신과 인터럽트 문자열 명령

NUCLEO-F401RE의 USART2와 ST-LINK Virtual COM Port를 이용해 PC 터미널 명령을 수신하고, 명령에 따라 LED 상태를 변경하는 과정을 정리한다.

## 기본 개념

UART(Universal Asynchronous Receiver/Transmitter)는 별도의 Clock 선 없이 TX와 RX 선으로 데이터를 주고받는 비동기 직렬 통신 방식이다. 송신 측과 수신 측이 같은 Baud rate와 Frame 형식을 사용해야 한다.

| 용어 | 의미 |
|---|---|
| TX | 데이터를 보내는 선 |
| RX | 데이터를 받는 선 |
| Baud rate | 1초 동안 전송하는 Symbol 수 |
| 8-N-1 | Data 8 bit, Parity 없음, Stop bit 1개 |
| Polling | 함수가 수신 완료까지 기다리는 방식 |
| Interrupt | 데이터가 도착했을 때 CPU에 이벤트를 알리는 방식 |

NUCLEO-F401RE에서는 USART2가 ST-LINK Virtual COM Port와 연결되어 있어 USB 케이블로 PC 터미널과 통신할 수 있다.

| 항목 | 설정 |
|---|---|
| UART | USART2 |
| TX / RX | PA2 / PA3 |
| Baud rate | 115200 bps |
| Frame | 8-N-1 |
| Flow control | None |

## 구현 목표

- PC 터미널로 시작 메시지와 현재 LED 상태를 송신한다.
- UART 문자를 Interrupt 방식으로 한 바이트씩 수신한다.
- 받은 문자를 32바이트 Buffer에 모으고 Enter로 명령을 완성한다.
- 문자열 명령으로 초록색, 파란색, 노란색, 빨간색 LED 상태를 변경한다.
- `status`, `help`, 잘못된 명령과 Buffer 초과를 처리한다.

## 핵심 코드

### `printf()`를 USART2에 연결

STM32에는 PC와 같은 기본 Console 출력 장치가 없다. `_write()`에서 `HAL_UART_Transmit()`을 호출하도록 연결하면 `printf()`가 만든 문자열을 USART2로 보낼 수 있다.

```c
int _write(int file, char *ptr, int len)
{
    (void)file;

    if (HAL_UART_Transmit(&huart2,
                          (uint8_t *)ptr,
                          (uint16_t)len,
                          HAL_MAX_DELAY) == HAL_OK)
    {
        return len;
    }

    return -1;
}
```

초기화가 끝나면 시작 메시지와 초기 상태를 출력한다.

```c
LED_ShowState(currentState);

printf("Hello STM32!\r\n");
UART_PrintState(currentState);
```

### 최초 Interrupt 수신 등록

`HAL_UART_Receive_IT()`에 한 바이트를 저장할 변수와 크기 `1U`를 전달한다. 등록 이후에는 문자가 들어올 때까지 `while (1)`이 멈추지 않는다.

```c
if (HAL_UART_Receive_IT(&huart2, &receivedByte, 1U) != HAL_OK)
{
    Error_Handler();
}
```

USART2 Interrupt가 발생하면 IRQ Handler가 제어를 HAL Driver에 전달한다.

```c
void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}
```

### 수신 Callback과 문자열 Buffer

한 바이트 수신이 끝날 때마다 `HAL_UART_RxCpltCallback()`이 호출된다. 일반 문자는 Buffer에 저장하고, Enter를 받으면 문자열 끝에 `\0`을 추가해 명령 완성 Flag를 설정한다.

```c
#define UART_RX_BUFFER_SIZE 32U

static uint8_t receivedByte;
static char uartRxBuffer[UART_RX_BUFFER_SIZE];
static volatile uint8_t uartRxIndex = 0U;
static volatile uint8_t uartCommandReady = 0U;
static volatile uint8_t uartRxOverflow = 0U;
```

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        if (uartCommandReady == 0U)
        {
            if ((receivedByte == '\r') || (receivedByte == '\n'))
            {
                if (uartRxIndex > 0U)
                {
                    uartRxBuffer[uartRxIndex] = '\0';
                    uartCommandReady = 1U;
                }
            }
            else if ((receivedByte == '\b') || (receivedByte == 0x7FU))
            {
                if (uartRxIndex > 0U)
                {
                    uartRxIndex--;
                }
            }
            else if (uartRxIndex < (UART_RX_BUFFER_SIZE - 1U))
            {
                uartRxBuffer[uartRxIndex] = (char)receivedByte;
                uartRxIndex++;
            }
            else
            {
                uartRxOverflow = 1U;
                uartCommandReady = 1U;
            }
        }

        HAL_UART_Receive_IT(&huart2, &receivedByte, 1U);
    }
}
```

마지막 `HAL_UART_Receive_IT()`는 다음 한 바이트를 받기 위한 재등록이다. 이 호출이 없으면 첫 글자를 받은 뒤 추가 수신이 진행되지 않는다.

### 완성된 명령 처리

Callback에서는 문자열을 모으는 짧은 작업만 수행한다. 완성된 명령의 비교, LED 제어와 UART 출력은 `while (1)`에서 처리한다.

```c
if (uartCommandReady != 0U)
{
    if (uartRxOverflow != 0U)
    {
        printf("Command too long\r\n");
    }
    else
    {
        UART_HandleCommand(uartRxBuffer);
    }

    uartRxIndex = 0U;
    uartRxOverflow = 0U;
    uartCommandReady = 0U;
}
```

`UART_HandleCommand()`는 `strcmp()`의 반환값이 `0`인지 확인해 명령을 구분한다.

| 명령 | 동작 |
|---|---|
| `g`, `G`, `led green` | 초록색 LED, `IDLE` |
| `b`, `B`, `led blue` | 파란색 LED, `RUNNING` |
| `y`, `Y`, `led yellow` | 노란색 LED, `WARNING` |
| `r`, `R`, `led red` | 빨간색 LED, `ERROR` |
| `status` | 현재 LED 상태 출력 |
| `help` | 사용할 수 있는 명령 출력 |
| 그 외 문자열 | `Unknown command. Type help.` 출력 |

## 코드 설명

### Interrupt 수신을 사용한 이유

Polling 수신은 데이터가 들어올 때까지 해당 함수에서 기다린다. 현재 코드는 Interrupt 수신을 사용하므로 UART 입력을 기다리는 동안에도 `while (1)`에서 Button Debouncing과 완성된 명령 처리를 계속 수행할 수 있다.

### 32바이트 Buffer에서 최대 31글자만 받는 이유

C 문자열은 마지막에 종료 문자 `\0`이 필요하다. 따라서 32바이트 배열의 마지막 한 칸을 `\0`용으로 남겨 두고, `uartRxIndex < UART_RX_BUFFER_SIZE - 1U`일 때만 문자를 저장한다.

### `volatile`을 사용한 이유

`uartCommandReady`, `uartRxIndex`, `uartRxOverflow`는 Interrupt Callback과 Main Loop가 함께 사용한다. `volatile`은 값이 실행 흐름 밖에서 바뀔 수 있으므로 매번 실제 메모리에서 읽도록 Compiler에 알린다.

### Callback에서 명령을 바로 처리하지 않는 이유

Interrupt Callback이 길어지면 다른 Interrupt 처리가 늦어질 수 있다. Callback은 수신 데이터 저장과 Flag 변경만 담당하고, `strcmp()`, LED 제어, `printf()`는 Main Loop에서 실행한다.

## 동작 흐름

```text
USART2 Interrupt 수신 등록
→ PC에서 문자 입력
→ USART2_IRQHandler()
→ HAL_UART_RxCpltCallback()
→ 문자를 Buffer에 저장
→ 다음 한 바이트 수신 재등록
→ Enter 입력 시 uartCommandReady = 1
→ Main Loop에서 문자열 명령 처리
→ LED 상태 변경 및 결과 송신
```

예를 들어 `led red`와 Enter를 입력하면 Buffer에는 `"led red\0"`이 만들어진다. `strcmp()`가 해당 명령을 찾으면 `currentState`를 `LED_STATE_ERROR`로 바꾸고 빨간색 LED를 켠다.

## 실행 결과와 확인 상태

Debug 빌드와 `02_UART.elf` 생성을 확인했다.

- RAM 사용량: 2.16%
- FLASH 사용량: 3.24%
- 프로젝트 기록에는 PC 터미널 송수신과 `led red` 명령의 LED 제어 확인이 남아 있다.
- `_write()`를 연결한 현재 코드의 `printf()` 터미널 출력은 실제 보드에서 다시 확인해야 한다.

```powershell
cmake --preset Debug
cmake --build build/Debug
```

## 배운 점

- USART2와 ST-LINK Virtual COM Port를 이용하면 별도 USB-UART Module 없이 PC와 통신할 수 있다.
- `HAL_UART_Receive_IT()`는 Main Loop를 막지 않고 한 바이트 수신을 등록한다.
- 수신 완료 Callback에서는 다음 수신을 반드시 다시 등록해야 한다.
- Enter를 기준으로 여러 바이트를 하나의 C 문자열 명령으로 만들 수 있다.
- Interrupt와 Main Loop가 공유하는 상태에는 `volatile`이 필요하다.
- `_write()`를 USART2 송신에 연결하면 기존 `printf()` 형식을 사용할 수 있다.

## 한 줄 정리

> USART2 Interrupt로 문자를 한 바이트씩 받아 문자열 명령을 만들고, Main Loop에서 명령에 맞는 LED 상태를 제어하도록 구현했다.
