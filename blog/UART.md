# STM32 UART: 송수신, 인터럽트, 문자열 명령

## 구현 목표

NUCLEO-F401RE의 USART2와 ST-LINK Virtual COM Port를 이용해 PC 터미널과 데이터를 주고받는다. UART 수신은 인터럽트 방식으로 처리하고, 한 글자씩 받은 데이터를 문자열 버퍼에 모아 LED 제어 명령으로 사용한다.

## USART2 설정

| 항목 | 설정 |
|---|---|
| UART | USART2 |
| TX | PA2 |
| RX | PA3 |
| Baud rate | 115200 bps |
| Data bits | 8 bit |
| Parity | None |
| Stop bits | 1 bit |
| Flow control | None |

```text
PC 터미널
↕ USB
ST-LINK Virtual COM Port
↕ USART2
PA2(TX) · PA3(RX)
↕
STM32F401RE
```

## UART 송신

초기화가 끝난 뒤 시작 메시지와 현재 LED 상태를 터미널로 출력한다.

```c
printf("Hello STM32!\r\n");
UART_PrintState(currentState);
```

`\r\n`은 터미널에서 줄을 바꿀 때 사용하는 Carriage Return과 Line Feed다.

## printf와 USART2 연결

STM32에는 PC와 같은 기본 콘솔이 없으므로 `_write()`를 USART2 송신 함수에 연결했다.

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

```text
printf()
→ _write()
→ HAL_UART_Transmit()
→ USART2
→ PC 터미널
```

이 연결을 통해 문자열과 변수를 간단하게 출력할 수 있다.

```c
printf("Command: %s\r\n", command);
printf("State: %s\r\n", stateName);
```

## UART 인터럽트 수신

문자 한 바이트를 인터럽트 방식으로 수신하도록 최초 수신을 등록한다.

```c
if (HAL_UART_Receive_IT(&huart2, &receivedByte, 1U) != HAL_OK)
{
    Error_Handler();
}
```

`HAL_UART_Receive_IT()`의 `IT`는 Interrupt를 의미한다. Polling처럼 반복문에서 계속 수신 상태를 확인하지 않고, 문자가 도착했을 때 UART 인터럽트가 발생한다.

```text
문자 도착
→ USART2_IRQHandler()
→ HAL_UART_IRQHandler()
→ HAL_UART_RxCpltCallback()
```

USART2 인터럽트는 NVIC에서 활성화한다.

```c
HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
HAL_NVIC_EnableIRQ(USART2_IRQn);
```

IRQ Handler는 발생한 USART2 인터럽트를 HAL에 전달한다.

```c
void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}
```

## 수신 완료 콜백과 문자열 버퍼

UART는 문자 한 바이트를 받을 때마다 `HAL_UART_RxCpltCallback()`을 호출한다. 콜백에서는 받은 문자를 버퍼에 저장하고 즉시 다음 수신을 다시 등록한다.

```c
#define UART_RX_BUFFER_SIZE 32U

static uint8_t receivedByte;
static char uartRxBuffer[UART_RX_BUFFER_SIZE];
static volatile uint8_t uartRxIndex = 0U;
static volatile uint8_t uartCommandReady = 0U;
```

```c
uartRxBuffer[uartRxIndex] = (char)receivedByte;
uartRxIndex++;
```

Enter에 해당하는 `\r` 또는 `\n`을 받으면 문자열 끝에 `\0`을 추가하고 명령 완성 상태를 표시한다.

```c
if ((receivedByte == '\r') || (receivedByte == '\n'))
{
    if (uartRxIndex > 0U)
    {
        uartRxBuffer[uartRxIndex] = '\0';
        uartCommandReady = 1U;
    }
}
```

```text
'l' → 'e' → 'd' → ' ' → 'r' → 'e' → 'd' → Enter
                         ↓
              "led red\0" 완성
```

버퍼는 32바이트이며 마지막 한 칸을 `\0`에 사용하므로 최대 31글자의 명령을 저장할 수 있다. 범위를 넘으면 `Command too long`을 출력한다.

## 문자열 명령 처리

완성된 문자열은 `strcmp()`로 비교한다. 두 문자열이 같으면 `strcmp()`가 `0`을 반환한다.

```c
if (strcmp(command, "led green") == 0)
{
    currentState = LED_STATE_IDLE;
}
else if (strcmp(command, "led red") == 0)
{
    currentState = LED_STATE_ERROR;
}
```

| 명령 | 동작 |
|---|---|
| `led green` | 초록색 LED, IDLE 상태 |
| `led blue` | 파란색 LED, RUNNING 상태 |
| `led yellow` | 노란색 LED, WARNING 상태 |
| `led red` | 빨간색 LED, ERROR 상태 |
| `status` | 현재 상태 출력 |
| `help` | 사용 가능한 명령 출력 |

한 글자 명령 `g`, `b`, `y`, `r`도 사용할 수 있으며 명령 뒤에 Enter를 입력해야 한다.

## 전체 동작 흐름

```text
USART2 인터럽트 수신 등록
→ 문자를 한 바이트씩 수신
→ 수신 완료 콜백 실행
→ 문자열 버퍼에 저장
→ Enter에서 명령 완성
→ while (1)에서 명령 처리
→ LED 상태 변경
→ printf()로 결과 출력
```

## 배운 점

- UART는 TX와 RX 선을 이용해 비동기 직렬 통신을 수행한다.
- `HAL_UART_Receive_IT()`를 사용하면 수신을 기다리며 반복문을 막지 않아도 된다.
- 수신 완료 콜백은 짧게 처리하고 완성된 명령은 `while (1)`에서 처리한다.
- 여러 문자를 배열에 모으고 마지막에 `\0`을 추가하면 C 문자열로 사용할 수 있다.
- `strcmp()`는 두 문자열이 같을 때 `0`을 반환한다.
- `_write()`를 USART2에 연결하면 `printf()` 출력을 UART 터미널로 보낼 수 있다.

> USART2 인터럽트로 문자를 한 바이트씩 받아 문자열 명령을 구성하고, LED 상태를 변경한 뒤 UART로 결과를 출력하도록 구현했다.
