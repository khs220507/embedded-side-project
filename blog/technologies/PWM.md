# STM32 PWM: CdS 기반 LED 밝기 제어

## 기본 개념

PWM(Pulse Width Modulation)은 Timer가 일정한 주기로 출력 핀을 HIGH와 LOW로 바꾸고, 한 주기에서 HIGH가 차지하는 비율인 Duty Cycle을 바꿔 평균 출력량을 조절하는 방식이다. LED는 Duty Cycle이 높을수록 더 밝게 보인다.

`06_PWM_LED`에서는 TIM1이 PWM 파형을 만들고, TIM2는 500ms마다 CdS 측정 시점만 만든다. 두 Timer는 역할이 다르다.

| Timer | 역할 | 실제 처리 |
|---|---|---|
| TIM2 | 측정 시점 생성 | 500ms Update Interrupt에서 Flag 설정 |
| TIM1 | LED 밝기 생성 | PA8(D7)에 1kHz PWM 출력 |

## 구현 목표

- PA8(D7)에 연결된 빨간 LED를 `TIM1_CH1` PWM으로 구동한다.
- CdS ADC 원시값 `0~4095`를 PWM Compare 값 `0~1000`으로 변환한다.
- 밝을 때는 낮은 Duty Cycle, 어두울 때는 높은 Duty Cycle로 LED 밝기를 바꾼다.
- UART로 CdS 값과 PWM Compare 값을 확인한다.

## 배선

기존 빨간 LED 배선을 그대로 사용한다.

```text
NUCLEO D7 (PA8) → 220Ω 저항 → 빨간 LED Anode(긴 다리)
빨간 LED Cathode(짧은 다리) → GND
```

PA8은 일반 GPIO 출력이 아니라 `TIM1_CH1` Alternate Function 출력으로 동작한다.

## 핵심 코드

### 1. TIM1 PWM 주파수와 Channel 설정

```c
#define PWM_PERIOD 999U

htim1.Instance = TIM1;
htim1.Init.Prescaler = 83;
htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
htim1.Init.Period = PWM_PERIOD;

channelConfig.OCMode = TIM_OCMODE_PWM1;
channelConfig.Pulse = 0;
channelConfig.OCPolarity = TIM_OCPOLARITY_HIGH;

HAL_TIM_PWM_ConfigChannel(&htim1, &channelConfig, TIM_CHANNEL_1);
HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
```

TIM1 Clock은 84MHz다. Prescaler `83`은 Counter Clock을 1MHz로 만들고, Period `999`는 1,000번의 Counter Tick으로 한 PWM 주기를 만든다.

```text
84MHz ÷ (83 + 1) = 1MHz
1MHz ÷ (999 + 1) = 1kHz PWM
```

### 2. PA8을 TIM1_CH1 출력으로 연결

```c
GPIO_InitStruct.Pin = RED_LED_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
HAL_GPIO_Init(RED_LED_GPIO_Port, &GPIO_InitStruct);
```

`GPIO_MODE_AF_PP`와 `GPIO_AF1_TIM1`은 PA8의 출력 제어권을 TIM1 Channel 1에 연결한다. 따라서 빨간 LED의 밝기는 이후 `HAL_GPIO_WritePin()`이 아니라 PWM Compare 값으로 제어한다.

### 3. ADC 값을 PWM Compare 값으로 변환

```c
static void PWM_SetDutyFromCdS(uint32_t rawValue)
{
  uint32_t pulse = (rawValue * (PWM_PERIOD + 1U)) / 4095U;

  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse);
}
```

`rawValue`는 12-bit ADC 값이므로 `0~4095`다. `pulse`는 `0~1000`으로 변환된다. `__HAL_TIM_SET_COMPARE()`는 TIM1을 멈추지 않고 Channel 1의 Compare Register를 갱신한다.

| CdS ADC 값 | PWM Compare 값 | LED 밝기 |
|---|---|---|
| `0` | `0` | 0%, 꺼짐 |
| 약 `2048` | 약 `500` | 약 50% |
| `4095` | `1000` | 100%, 최대 밝기 |

## 전체 동작 흐름

```text
TIM2 Update Interrupt 발생 (500ms)
  ↓
HAL_TIM_PeriodElapsedCallback()
  ↓
timerSampleRequested = 1
  ↓
Main Loop가 Flag 확인
  ↓
ADC1에서 CdS rawValue 측정
  ↓
rawValue를 pulse(0~1000)로 변환
  ↓
__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse)
  ↓
TIM1_CH1(PA8)의 Duty Cycle 변경
  ↓
빨간 LED 밝기 변경 + UART 출력
```

Interrupt Callback은 Flag만 설정한다. ADC Polling, PWM 계산, UART 출력처럼 시간이 걸릴 수 있는 작업은 Main Loop에서 실행한다.

## 실행 결과

| 항목 | 상태 |
|---|---|
| 구현 | 완료 |
| Debug 빌드 | 완료 — FLASH 26,716 B / 512 KB, RAM 2,696 B / 96 KB |
| NUCLEO-F401RE Flash·Verify·Reset | 완료 |
| PA8 빨간 LED 밝기 변화·UART 출력 관찰 | 보드 검증 완료 — 밝음·어두움 조건에서 함께 변화 확인 |

## 주의 사항

PA8은 PWM 출력 전용 핀이다. 이전 프로젝트의 `LED_ShowState()` 또는 UART `led red` 명령처럼 GPIO로 PA8을 제어하던 코드는 PWM 출력을 직접 바꾸지 못한다. 빨간 LED의 밝기는 `PWM_SetDutyFromCdS()`가 관리한다.

## 배운 점

- PWM의 밝기는 Counter Period와 Compare 값의 비율로 결정된다.
- PWM 출력에는 Timer Alternate Function 핀 설정이 필요하다.
- ADC와 PWM의 값 범위가 다르면 목적에 맞는 범위로 변환해야 한다.
- Timer Interrupt는 작업 시점만 알리고 실제 처리는 Main Loop로 분리할 수 있다.

## 한 줄 정리

**TIM2가 500ms마다 CdS 측정을 요청하고, Main Loop가 ADC 값을 TIM1_CH1의 Duty Cycle로 바꿔 PA8 빨간 LED 밝기를 자동 제어한다.**
