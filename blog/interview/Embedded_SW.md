# STM32 Embedded SW: 면접 핵심 개념

프로젝트에서 실제로 사용한 기술을 바탕으로, 코드 설명보다 면접 답변에 필요한 개념과 판단 기준을 정리한다.

## Interrupt와 Main Loop

Interrupt는 발생한 사건에 빠르게 반응해야 하므로 처리 시간을 짧게 유지한다. 긴 연산, 통신, 대기 작업을 ISR 안에서 수행하면 다른 Interrupt가 늦어질 수 있다. 따라서 ISR은 Flag 또는 이벤트만 전달하고, 실제 작업은 Main Loop에서 처리하는 구조가 기본이다.

프로젝트 사례: TIM2 Callback은 측정 요청 Flag만 설정하고 ADC·PWM·UART 처리는 Main Loop에서 수행했다.

## `volatile`

Interrupt, DMA, 하드웨어 Register처럼 프로그램 흐름 밖에서 값이 바뀔 수 있는 데이터를 컴파일러가 항상 다시 읽도록 하는 키워드다. 다만 `volatile`은 원자성이나 여러 변수의 동기화를 보장하지 않는다.

프로젝트 사례: Timer 및 UART DMA Callback과 Main Loop가 공유하는 상태 Flag에 사용했다.

## DMA와 `ReceiveToIdle`

DMA는 CPU 대신 Peripheral과 메모리 사이의 데이터 이동을 처리한다. CPU는 전송이 끝났다는 이벤트만 받아 다른 작업을 계속할 수 있다. `ReceiveToIdle`은 수신 길이가 정해지지 않은 문자열·명령에 적합하며, 전송이 멈춘 Idle 시점까지 받은 데이터를 처리한다.

프로젝트 사례: USART2 RX/TX DMA로 명령 수신과 `printf()` 출력을 처리했다.

## ADC Polling과 비동기 방식

Polling은 CPU가 변환 완료를 확인하는 단순한 방식이다. 측정 주기가 느리고 변환 시간이 짧다면 적합하다. 빠른 연속 측정 또는 동시에 처리할 일이 많으면 CPU 대기를 줄이기 위해 ADC Interrupt 또는 DMA를 고려한다.

프로젝트 사례: CdS를 500ms 주기로 읽으므로 ADC Polling을 선택했다.

## Timer의 역할 분리

Timer는 주기 작업의 기준 시간을 만들거나 PWM 같은 하드웨어 파형을 생성한다. 주기가 다른 기능을 하나의 Timer에 섞으면 설정과 유지보수가 복잡해지므로 역할에 따라 분리하는 편이 좋다.

프로젝트 사례: TIM2는 센서 측정 주기, TIM1은 LED PWM 출력을 담당했다.

## PWM

PWM은 주파수를 유지한 채 한 주기 중 HIGH 시간 비율(Duty Cycle)을 바꿔 평균 출력량을 제어한다. LED 밝기는 깜빡이는 횟수가 아니라 Duty Cycle에 따라 달라진다. PWM 주파수는 `Timer Clock / ((Prescaler + 1) × (Period + 1))`로 계산한다.

프로젝트 사례: TIM1 1kHz PWM의 Compare 값을 CdS ADC 값에 따라 바꿔 LED 밝기를 조절했다.

## Alternate Function

GPIO 핀은 단순 입력·출력뿐 아니라 UART, I2C, Timer 같은 Peripheral 신호를 핀으로 연결하는 Alternate Function 역할을 가질 수 있다. Peripheral 출력을 사용하려면 해당 핀과 Alternate Function 번호가 MCU 데이터시트와 일치해야 한다.

프로젝트 사례: PA8을 `GPIO_AF1_TIM1`으로 설정해 TIM1 Channel 1 PWM을 출력했다.

## 빌드와 보드 검증

빌드 성공은 코드와 링크 설정이 유효하다는 뜻이다. 실제 동작에는 배선, 전원, 핀 설정, 센서 주소, 통신 속도까지 영향을 주므로 보드 검증을 별도로 해야 한다.

프로젝트 사례: 각 챕터에서 구현·빌드·Flash/Verify·실제 동작 확인을 구분해 기록했다.
