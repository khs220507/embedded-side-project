# STM32 ADC: CdS 조도 센서 측정

## 기본 개념

ADC는 `Analog-to-Digital Converter`의 약자로, 연속적인 전압을 MCU가 처리할 수 있는 정수값으로 변환한다. `04_ADC_CdS`에서는 CdS 조도 센서 모듈의 Analog Output을 NUCLEO-F401RE의 PA0(`ADC1_IN0`, Arduino A0)에서 읽는다.

12-bit ADC 결과 범위는 0~4095다. 현재 코드는 기준 전압을 3300mV로 가정해 다음 식으로 전압을 계산한다.

```text
전압(mV) = ADC 원시값 × 3300 ÷ 4095
```

이 값은 이해하기 쉬운 환산값이며, 실제 VDDA 전압과 ADC 오차를 보정한 정밀 측정값은 아니다.

## 구현 목표

- PA0를 `ADC1_IN0` Analog Input으로 사용한다.
- ADC1을 12-bit 단일 변환, Software Trigger 방식으로 설정한다.
- 500ms마다 CdS 값을 한 번 측정한다.
- 원시값과 환산 전압을 USART2 DMA 기반 `printf()`로 출력한다.

## 배선

| CdS 모듈 | NUCLEO-F401RE | 역할 |
|---|---|---|
| `VCC` | `3.3V` | 센서 모듈 전원 |
| `GND` | `GND` | 공통 기준 전압 |
| `AO` | `A0` / `PA0` | ADC1 Channel 0 입력 |

STM32F401RE의 ADC 입력에 3.3V를 넘는 전압이 들어가지 않도록 센서 모듈도 3.3V로 구동한다. 모듈에 `DO`가 있더라도 이번 구현에서는 사용하지 않는다.

## 핵심 코드

### ADC1 초기화

```c
hadc1.Instance = ADC1;
hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
hadc1.Init.Resolution = ADC_RESOLUTION_12B;
hadc1.Init.ScanConvMode = DISABLE;
hadc1.Init.ContinuousConvMode = DISABLE;
hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
hadc1.Init.NbrOfConversion = 1;

channelConfig.Channel = ADC_CHANNEL_0;
channelConfig.Rank = 1;
channelConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;
```

APB2가 84MHz이므로 `ADC_CLOCK_SYNC_PCLK_DIV4`를 적용한 ADC Clock은 21MHz다. 한 Channel만 읽기 때문에 Scan과 Continuous Conversion을 끄고, 매 측정 시점에 Software로 변환을 시작한다.

### 단일 값 읽기

```c
static HAL_StatusTypeDef ADC_ReadCdS(uint32_t *rawValue)
{
    HAL_StatusTypeDef status = HAL_ADC_Start(&hadc1);

    if (status != HAL_OK)
    {
        return status;
    }

    status = HAL_ADC_PollForConversion(&hadc1, 10U);
    if (status == HAL_OK)
    {
        *rawValue = HAL_ADC_GetValue(&hadc1);
    }

    if (HAL_ADC_Stop(&hadc1) != HAL_OK && status == HAL_OK)
    {
        status = HAL_ERROR;
    }

    return status;
}
```

`HAL_ADC_Start()`가 변환을 시작하고 `HAL_ADC_PollForConversion()`이 완료를 기다린다. 성공했을 때만 `HAL_ADC_GetValue()`로 결과를 읽고, 마지막에 ADC를 정지한다.

### 500ms 주기 측정과 UART 출력

```c
uint32_t now = HAL_GetTick();

if ((now - lastSensorSampleTime) >= 500U)
{
    uint32_t rawValue;
    lastSensorSampleTime = now;

    if (ADC_ReadCdS(&rawValue) == HAL_OK)
    {
        uint32_t millivolts = (rawValue * 3300U) / 4095U;
        currentState = CdS_GetBrightnessState(rawValue);
        LED_ShowState(currentState);
        printf("CdS raw=%lu, voltage=%lu mV, brightness=%s\r\n",
               (unsigned long)rawValue,
               (unsigned long)millivolts,
               CdS_GetBrightnessName(currentState));
    }
}
```

`HAL_Delay()` 대신 `HAL_GetTick()`의 차이를 확인하므로 기존 버튼 입력과 UART 명령 처리를 계속 실행할 수 있다.

## 동작 흐름

```text
CdS 모듈 AO
→ PA0 / ADC1_IN0
→ ADC 변환 시작
→ 완료 Polling
→ 0~4095 원시값
→ mV 환산
→ USART2 출력
```

기본 출력 형식은 다음과 같다.

```text
CdS raw=252, voltage=203 mV, brightness=BRIGHT
```

## 조도 단계 LED 표시

밝을 때 `raw=252~308`, 어두울 때 `raw=3657~3881`을 실제로 측정했다. 두 끝값 사이를 네 구간으로 나누어 현재 4색 LED 회로에 연결했다. 이 모듈은 ADC 값이 클수록 어두운 상태다.

| ADC 원시값 | 상태 문자열 | LED |
|---:|---|---|
| 0~1200 | `BRIGHT` | 초록 |
| 1201~2100 | `NORMAL` | 파랑 |
| 2101~3000 | `DIM` | 노랑 |
| 3001~4095 | `DARK` | 빨강 |

```c
currentState = CdS_GetBrightnessState(rawValue);
LED_ShowState(currentState);
```

500ms마다 새 ADC 값으로 LED 상태를 갱신하므로, 이전 UART LED 명령이나 버튼으로 바꾼 LED는 다음 센서 측정 때 조도 상태로 다시 바뀐다.

## 현재 검증 상태

- 구현 완료: ADC1 Channel 0 초기화, 500ms Polling 측정, UART 출력
- 빌드 완료: `04_ADC_CdS.elf` 생성 확인
- Flash·Verify: 완료
- 기본 UART 측정: 완료 (`CdS raw=587~588, voltage=473 mV`)
- 어두운 상태: 완료 (`raw=3657~3881`, `2.947~3.127V`)
- 밝은 상태: 완료 (`raw=252~308`, `203~248mV`)
- 조도 단계 LED 표시: Flash·Verify·실제 LED·UART 출력 검증 완료

기본 측정에서는 `raw=587~588`, `473mV`가 500ms 주기로 출력되는 것을 확인했다. 센서를 가린 어두운 상태에서는 `raw=3657~3881`, `2.947~3.127V`, 휴대폰 플래시를 비춘 밝은 상태에서는 `raw=252~308`, `203~248mV`를 확인했다. 이 모듈은 어두울수록 `AO` 전압과 ADC 원시값이 커지는 방향이다.

조도 단계 표시 버전도 실제 NUCLEO-F401RE에서 검증했다. 밝은 상태에서는 `brightness=BRIGHT`와 초록 LED, 어두운 상태에서는 `brightness=DARK`와 빨강 LED가 일치했다.

## 배운 점

- Analog Input은 HIGH/LOW 두 상태가 아니라 전압 범위를 수치로 표현한다.
- 12-bit ADC는 입력 범위를 4096단계로 나눈다.
- Channel은 물리 입력 경로이고 Rank는 변환 순서다.
- Polling 변환은 구조가 단순하지만 완료될 때까지 CPU가 기다린다.
- 센서별 출력 방향과 범위를 실제로 측정한 뒤 임계값을 정해야 한다.

## 한 줄 정리

PA0의 CdS Analog Output을 ADC1의 12-bit 값으로 변환하고, 500ms마다 원시값과 환산 전압을 UART로 확인한다.
