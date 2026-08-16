# STM32 Embedded SW: 면접 핵심 정리

현재 프로젝트에서 실제로 구현하고 검증한 내용 중 면접에서 설명할 가치가 큰 주제만 정리한다. 새 챕터를 마칠 때마다 핵심 질문을 이 문서에 추가한다.

## 전체 데이터 흐름

`06_PWM_LED`에서는 TIM2가 500ms마다 측정 요청을 만들고, Main Loop가 ADC1으로 CdS를 읽은 뒤 TIM1 PWM Duty Cycle을 바꾼다. UART DMA는 상태와 측정값을 출력한다.

```text
TIM2 Update Interrupt
  → volatile Flag 설정
  → Main Loop의 ADC Polling
  → ADC 값을 PWM Compare 값으로 변환
  → TIM1_CH1(PA8) Duty Cycle 갱신
  → UART DMA로 결과 출력
```

## Interrupt와 Main Loop의 역할 분리

### Q. Interrupt Callback에서 ADC 변환과 UART 출력까지 모두 처리하지 않은 이유는 무엇인가?

Interrupt는 짧게 끝나야 다른 Interrupt의 지연을 줄일 수 있다. 그래서 `HAL_TIM_PeriodElapsedCallback()`에서는 `timerSampleRequested = 1U`만 설정하고, 시간이 더 걸리는 ADC Polling·PWM 계산·UART 출력은 Main Loop가 처리했다.

### Q. Interrupt와 Main Loop가 함께 사용하는 Flag에 `volatile`이 필요한 이유는 무엇인가?

`timerSampleRequested`는 Interrupt가 쓰고 Main Loop가 읽는다. `volatile`을 붙이면 컴파일러가 Main Loop에서 값을 한 번만 읽은 것으로 최적화하지 않고, 매번 메모리의 최신 값을 읽도록 한다. 다만 `volatile`만으로 복잡한 공유 데이터의 원자성이나 동기화가 보장되지는 않는다.

## DMA 기반 UART

### Q. DMA를 UART에 사용한 이유는 무엇인가?

DMA는 CPU가 바이트마다 전송 레지스터를 처리하지 않아도 메모리와 UART Peripheral 사이의 전송을 수행한다. 이 프로젝트에서는 UART RX/TX DMA와 `ReceiveToIdle`을 사용해, Main Loop가 센서 처리와 상태 제어를 계속할 수 있게 했다.

### Q. `ReceiveToIdle`은 언제 유용한가?

수신 길이가 고정되지 않은 명령 문자열에 유용하다. 지정한 버퍼 크기만큼 모두 채워질 때까지 기다리지 않고, UART의 Idle 상태가 발생하면 현재까지 받은 데이터를 Callback으로 전달할 수 있다.

## ADC와 센서 값 처리

### Q. ADC Polling 방식을 선택한 이유와 한계는 무엇인가?

CdS는 500ms 주기로 한 번 읽으므로 `HAL_ADC_Start()` 뒤 `HAL_ADC_PollForConversion()`으로 완료를 기다리는 방식으로도 충분했다. 다만 변환이 길거나 다른 실시간 작업이 많아지면 CPU가 기다리는 시간이 생기므로 ADC Interrupt 또는 DMA 방식을 검토해야 한다.

### Q. ADC 값과 실제 전압은 어떻게 변환했는가?

STM32F401RE의 12-bit ADC 원시값 범위는 `0~4095`이고, 기준 전압을 3.3V로 두었다. 따라서 전압은 `rawValue * 3300 / 4095` mV로 계산했다. 실제 측정값이 전압과 어떤 방향으로 변하는지는 CdS 분압 회로의 연결 방향까지 함께 확인해야 한다.

## Timer와 PWM

### Q. TIM1과 TIM2의 역할을 분리한 이유는 무엇인가?

TIM1은 PA8의 `TIM1_CH1`에서 1kHz PWM을 계속 출력한다. TIM2는 500ms마다 Update Interrupt를 발생시켜 센서 측정 시점만 만든다. 주기적인 측정 작업과 고속 PWM 출력을 하나의 Timer에 섞지 않아 설정과 동작을 분명하게 분리했다.

### Q. PWM으로 LED 밝기를 바꾸는 원리는 무엇인가?

PWM 주파수는 1kHz로 고정하고, 한 주기 중 HIGH인 시간의 비율인 Duty Cycle을 바꾼다. 현재 TIM1의 `Period`는 999이므로 한 주기는 1000카운트이며, `__HAL_TIM_SET_COMPARE()`가 Compare 값을 바꿔 HIGH 시간을 변경한다. 밝기를 조절하는 값은 깜빡이는 횟수가 아니라 Duty Cycle이다.

### Q. 현재 PWM 주파수는 어떻게 계산하는가?

TIM1 Clock은 84MHz, Prescaler는 83, Period는 999다. 따라서 Counter Clock은 `84MHz / (83 + 1) = 1MHz`이고, PWM 주파수는 `1MHz / (999 + 1) = 1kHz`다.

## Peripheral 핀 연결과 디버깅

### Q. PA8을 일반 GPIO가 아닌 Alternate Function으로 설정한 이유는 무엇인가?

PA8을 `GPIO_AF1_TIM1`으로 설정해야 TIM1 Channel 1 하드웨어가 핀을 직접 제어할 수 있다. 일반 GPIO 출력 설정은 `HAL_GPIO_WritePin()` 같은 소프트웨어 제어용이며, Timer PWM 파형을 핀으로 내보낼 수 없다.

### Q. 임베디드 기능을 검증할 때 빌드와 보드 검증을 구분하는 이유는 무엇인가?

빌드 성공은 소스와 링크 설정이 유효하다는 의미일 뿐, 배선·핀 Alternate Function·전원·센서 방향·통신 연결까지 보장하지는 않는다. 그래서 이 프로젝트는 구현 완료, 빌드 완료, Flash·Verify, 실제 LED·UART 동작 확인을 별도 상태로 기록했다.
