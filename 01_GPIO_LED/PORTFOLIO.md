# STM32 GPIO 학습 기록

**기간:** 2026.08.13 ~ 진행 중  
**환경:** STM32 NUCLEO-F401RE, C, STM32 HAL, STM32CubeMX, CMake, ST-LINK

## 학습 내용

- GPIO 입력과 출력, 포트·핀의 개념 학습
- `HAL_GPIO_ReadPin()`을 이용한 버튼 입력 확인
- `HAL_GPIO_WritePin()`을 이용한 LED ON/OFF 제어
- Push-Pull 출력과 HIGH/LOW 신호의 동작 이해
- CMake 빌드, 펌웨어 다운로드 및 실제 보드 검증 과정 경험

## 구현 내용

B1 버튼(PC13)을 누르면 내장 LED(PA5)와 외부 초록 LED(PB5)가 켜지고, 버튼을 놓으면 꺼지도록 구현했다. 부팅 시 버튼의 평상시 값을 저장해 현재 입력과 비교함으로써 버튼의 Active High/Low 차이에 대한 의존성을 줄였다.

## 이력서 한 줄 요약

STM32 HAL 기반 GPIO 버튼·LED 제어를 구현하고, CMake 빌드부터 ST-LINK 펌웨어 다운로드 및 하드웨어 동작 검증까지 수행
