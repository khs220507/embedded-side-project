# Embedded Systems Project Roadmap

> **Goal:** C# / WPF application developer → Embedded Systems developer\
> **Board:** STM32 NUCLEO-F401RE\
> **Direction:** WPF upper-level control + STM32 lower-level control

## 이 로드맵에서 배울 수 있는 기술 스택

| 분야 | 기술 스택 | 학습 내용 |
|---|---|---|
| Firmware | Embedded C / C++ | MCU 프로그램 구조, 상태 관리, 센서·모터 제어 로직 |
| STM32 | STM32CubeIDE, STM32CubeMX, STM32 HAL | Clock·Pin·Peripheral 설정과 HAL 기반 Firmware 구현 |
| MCU Peripheral | GPIO, ADC, UART, I2C, Timer, PWM, DMA, Interrupt | LED·버튼·센서 입력·주기 작업·비동기 통신 제어 |
| Sensor / Actuator | CdS, DHT11, MPU6050, HC-SR04, IR Sensor, Servo, DC Motor, Stepper Motor | 실제 환경 데이터 측정과 모터 구동 |
| RTOS | FreeRTOS | Task, Queue, Semaphore 기반 동시 작업 처리 |
| PC Application | C#, .NET, WPF | STM32 Serial 통신, 모니터링 화면, 제어 UI 구성 |
| System Design | Serial Protocol, State Machine, Sensor-Actuator Integration | 상위 WPF와 하위 MCU를 연결한 제어 시스템 설계 |
| Development Tools | Git, GitHub, CMake, VS Code | 소스 관리, 프로젝트 빌드, 학습 기록 관리 |

**최종적으로는 WPF가 상태를 표시·명령하고, STM32가 센서와 모터를 실시간으로 제어하는 임베디드 제어 시스템을 구현한다.**

### 핵심 학습 기술 키워드

`GPIO` · `EXTI` · `ADC` · `UART / USART` · `I2C` · `Timer` · `PWM` · `DMA` · `Interrupt / NVIC` · `FreeRTOS` · `Sensor Interface` · `Motor Control` · `Serial Protocol` · `C# / WPF`

현재처럼 한 챕터씩 실제 구현하면서 각 기술의 초기화, 데이터 흐름, IRQ·Callback 연결, 보드 검증까지 익힌다.

------------------------------------------------------------------------

## 0. Development Environment

### Hardware

-   STM32 NUCLEO-F401RE
-   Breadboard
-   Jumper wires
-   LEDs
-   Resistors
-   HC-SR04
-   MPU6050
-   DHT11
-   CdS light sensor
-   IR obstacle sensor
-   SG90 servo motor
-   N20 DC geared motor
-   TB6612FNG motor driver
-   28BYJ-48 stepper motor
-   ULN2003 stepper motor driver

### Software

-   C / C++
-   STM32CubeIDE or VS Code
-   STM32 HAL
-   C# / .NET
-   WPF
-   Git

------------------------------------------------------------------------

# Level 1. GPIO

## Project 01 --- LED Status Controller

### Goal

Learn the basic structure of STM32 firmware and GPIO control.

### Features

-   LED ON / OFF
-   Multiple LED control
-   Status indication

Example:

``` text
Green  = Idle
Blue   = Running
Yellow = Warning
Red    = Error
```

### Learn

-   GPIO Output
-   GPIO Input
-   STM32 HAL
-   MCU Pin configuration
-   Basic firmware structure

------------------------------------------------------------------------

# Level 2. Sensors

## Project 02 --- Sensor Monitoring Device

### Goal

Read real-world sensor data using STM32.

### Sensors

-   DHT11 --- Temperature / Humidity
-   CdS --- Light
-   HC-SR04 --- Distance
-   IR Sensor --- Object detection

### Learn

-   Digital Input
-   ADC
-   Timer
-   Sensor sampling
-   Interrupt basics

### Architecture

``` text
Sensors
   ↓
STM32
   ↓
Sensor Data
```

------------------------------------------------------------------------

# Level 3. STM32 ↔ WPF Communication

## Project 03 --- WPF Device Monitor

### Goal

Connect embedded firmware with the existing C# / WPF skill set.

### Architecture

``` text
Sensors
   ↓
STM32
   ↓
UART / Serial
   ↓
C# WPF
   ↓
Monitoring Dashboard
```

### WPF Features

-   Serial Port connection
-   Device connection status
-   Temperature display
-   Humidity display
-   Light level display
-   Distance display
-   Real-time chart
-   Log window

### Command Example

``` text
DATA,TEMP,24.5
DATA,HUM,52
DATA,DIST,35.2

CMD,LED,RED,ON
CMD,LED,RED,OFF
```

### Learn

-   UART
-   Serial communication
-   Packet design
-   Parsing
-   Command / Response
-   Device ↔ PC communication

------------------------------------------------------------------------

# Level 4. Motor Control

## Project 04 --- WPF Motor Controller

### Hardware

-   STM32 NUCLEO-F401RE
-   TB6612FNG
-   N20 DC Motor

### Architecture

``` text
WPF
 ↓
UART
 ↓
STM32
 ↓
PWM
 ↓
TB6612FNG
 ↓
DC Motor
```

### Features

-   Start
-   Stop
-   Forward
-   Reverse
-   Speed control

### WPF UI

``` text
Motor Status : RUNNING

Direction
[ Forward ] [ Reverse ]

Speed
[----------●-----] 70%

[ START ] [ STOP ]
```

### Learn

-   PWM
-   Timer
-   Motor Driver
-   Motor direction control
-   PC-based device control

------------------------------------------------------------------------

# Level 5. Sensor + Actuator

## Project 05 --- Radar Scanner

### Hardware

-   HC-SR04
-   SG90 Servo
-   STM32

### Architecture

``` text
SG90 Servo
    +
HC-SR04
    ↓
STM32
    ↓
UART
    ↓
WPF Radar UI
```

### Features

-   Servo scanning
-   Distance measurement
-   Angle measurement
-   Object detection
-   WPF radar visualization

### Learn

-   Servo PWM
-   HC-SR04 timing
-   Sensor + actuator integration
-   Real-time visualization

------------------------------------------------------------------------

# Level 6. Position Control

## Project 06 --- Stepper Motor Position Controller

### Hardware

-   28BYJ-48
-   ULN2003
-   STM32

### Features

-   Jog +
-   Jog -
-   0°
-   45°
-   90°
-   180°
-   Home
-   Position display

### WPF Example

``` text
Current Position : 90°

[ HOME ]

[ JOG - ] [ JOG + ]

[ 0° ] [ 45° ] [ 90° ] [ 180° ]
```

### Learn

-   Stepper motor control
-   Position control
-   Sequence control
-   State management

------------------------------------------------------------------------

# Level 7. FreeRTOS

## Project 07 --- FreeRTOS Device Controller

### Goal

Convert the previous firmware into a multitasking embedded system.

### Task Structure

``` text
SensorTask
   ├─ Read sensors
   └─ Update sensor data

MotorTask
   ├─ Motor control
   └─ Position control

CommunicationTask
   ├─ UART RX
   ├─ UART TX
   └─ Command parsing

MonitoringTask
   ├─ Device status
   └─ Error monitoring
```

### Learn

-   Task
-   Scheduler
-   Queue
-   Semaphore
-   Mutex
-   Task Priority
-   Inter-task communication

### C# Knowledge Connection

``` text
C# / .NET             FreeRTOS

Thread        →       Task
Task          →       Task
lock          →       Mutex
Semaphore     →       Semaphore
ConcurrentQueue
              →       Queue
CancellationToken
              →       Task control / event
```

------------------------------------------------------------------------

# Level 8. Final Project

## Project 08 --- Industrial Embedded Control System

### Goal

Build a small industrial control system combining WPF and STM32.

### Architecture

``` text
┌──────────────────────────────┐
│          C# WPF HMI          │
│                              │
│ Dashboard                    │
│ Real-time Chart              │
│ Motor Control                │
│ Alarm History                │
│ Device Status                │
│ Log                          │
└──────────────┬───────────────┘
               │
          UART / Serial
               │
┌──────────────▼───────────────┐
│        STM32 F401RE          │
│                              │
│ FreeRTOS                     │
│ Communication Task           │
│ Sensor Task                  │
│ Motor Task                   │
│ Monitoring Task              │
└───────┬──────────┬───────────┘
        │          │
     Sensors     Motors
```

### STM32

-   Sensor acquisition
-   Motor control
-   Device state management
-   Error detection
-   Communication
-   FreeRTOS task management

### WPF

-   Device connection
-   Real-time monitoring
-   Real-time chart
-   Motor control
-   Alarm display
-   Event history
-   Logging
-   Device configuration

------------------------------------------------------------------------

# Next Step

After completing the UART version:

``` text
UART
 ↓
RS-485
 ↓
CAN
```

Then expand toward:

``` text
WPF HMI
   ↓
CAN / RS-485
   ↓
STM32
   ↓
Sensors / Actuators
```

This structure is closer to industrial automation, robotics, automotive,
and defense embedded systems.

------------------------------------------------------------------------

# Recommended Order

``` text
01. GPIO / LED
       ↓
02. Sensor
       ↓
03. UART + WPF
       ↓
04. Motor Control
       ↓
05. Sensor + Actuator
       ↓
06. Position Control
       ↓
07. FreeRTOS
       ↓
08. Industrial Embedded Control System
       ↓
09. CAN / RS-485
```

------------------------------------------------------------------------

# GitHub Repository Structure

``` text
embedded-systems-projects/
│
├── 01_GPIO_LED/
├── 02_Sensor_Monitor/
├── 03_UART_WPF_Device_Monitor/
├── 04_WPF_Motor_Controller/
├── 05_Radar_Scanner/
├── 06_Stepper_Position_Controller/
├── 07_FreeRTOS_Device_Controller/
└── 08_Industrial_Control_System/
```

------------------------------------------------------------------------

## Final Goal

**C# / WPF Application Developer**

↓

**WPF + Device Communication Developer**

↓

**STM32 Firmware + WPF HMI Developer**

↓

**Embedded Systems / Industrial Control Software Developer**
