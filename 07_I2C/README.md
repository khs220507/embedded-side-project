# Project 07 - I2C MPU6050

MPU6050을 I2C1으로 연결해 I2C Master 통신과 센서 Register 읽기를 학습하는 프로젝트다.

## 계획

- I2C1 Master를 PB8(D15, SCL), PB9(D14, SDA)에 설정
- MPU6050의 기본 주소 `0x68` 응답 확인
- `WHO_AM_I` Register를 읽어 장치 식별
- UART로 통신 결과 출력

## 현재 상태

- 프로젝트 폴더 생성 및 이전 챕터 기반 복제 완료
- I2C Peripheral 설정·통신 구현·빌드·보드 검증은 아직 시작 전
