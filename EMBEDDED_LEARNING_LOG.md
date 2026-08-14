# 임베디드 학습 기록

STM32를 중심으로 임베디드 시스템을 공부하며 직접 구현하고 확인한 내용을 날짜별로 기록한다.

## 학습 진행 현황

| 단계 | 주제 | 상태 |
|---:|---|---|
| 01 | GPIO 및 LED 제어 | 진행 중 |
| 02 | 센서 데이터 수집 | 예정 |
| 03 | UART 및 WPF 통신 | 예정 |
| 04 | 모터 제어 | 예정 |
| 05 | FreeRTOS | 예정 |

## 기록 목차

- [2026.08.13 — GPIO 버튼·LED 제어](#20260813--gpio-버튼led-제어)

---

## 2026.08.13 — GPIO 버튼·LED 제어

### 학습 목표

- GPIO 입력과 출력의 기본 동작 이해
- 사용자 버튼으로 내장·외부 LED 제어
- 빌드부터 보드 동작 확인까지 전체 과정 경험

### 핵심 개념

- GPIO 핀은 포트와 핀 번호를 함께 사용한다. PA5와 PB5는 서로 다른 핀이다.
- `HAL_GPIO_ReadPin()`으로 버튼의 HIGH/LOW 입력을 읽는다.
- `HAL_GPIO_WritePin()`으로 LED 출력을 SET/RESET한다.
- LED 출력에는 Push-Pull 방식을 사용했다.

### 실습 내용

| 부품 | 핀 | 역할 |
|---|---|---|
| B1 사용자 버튼 | PC13 | 입력 |
| 내장 LED LD2 | PA5 | 출력 |
| 외부 초록 LED | PB5 | 출력 |

```c
if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) != button_released_state)
{
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_SET);
}
else
{
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
}
```

부팅할 때 버튼의 평상시 입력값을 저장하고 현재 값과 비교했다. 버튼 입력의 Active High/Low를 고정하지 않고 눌림 상태를 판단하기 위한 방식이다.

### 실습 결과

- CMake Debug 빌드 및 ELF 파일 생성 성공
- ST-LINK를 통한 펌웨어 다운로드 성공
- B1 버튼을 누르면 내장 LED와 외부 초록 LED가 켜지는 동작 확인
- 버튼을 놓으면 두 LED가 꺼지는 동작 확인

### 배운 점

GPIO 제어는 핀 초기화와 HAL 함수 사용뿐 아니라 입력 신호의 기준 상태를 올바르게 해석하는 것도 중요하다. 또한 빌드 성공에서 끝내지 않고 실제 보드에 다운로드하여 하드웨어 동작까지 확인해야 한다.

### 다음 학습

- PB5 초록 LED 1초 주기 점멸
- 네 개의 외부 LED 개별 제어
- 버튼 디바운싱과 상태 전환 구현

---

## 새 기록 작성 양식

아래 양식을 복사해 문서 위쪽 기록 다음에 추가한다.

```markdown
## YYYY.MM.DD — 학습 주제

### 학습 목표

-

### 핵심 개념

-

### 실습 내용


### 실습 결과

-

### 문제와 해결

- 문제:
- 원인:
- 해결:

### 배운 점


### 다음 학습

-
```
