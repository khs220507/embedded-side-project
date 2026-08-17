# Project 09 - Data Structures Basic

`DS-01` 배열·문자열 버퍼를 C 코드로 구현하고 테스트하는 학습 챕터다. STM32 Peripheral을 새로 설정하지 않으며, 다음 단계에서 `03_UART_DMA`의 수신 버퍼와 연결한다.

## 구현 목표

- 고정 크기 `char` 배열로 명령 문자열을 저장한다.
- 현재 길이를 `length`로 관리한다.
- `\n`을 만나면 문자열 끝에 `\0`을 기록하고 명령 완료를 알린다.
- 버퍼 끝을 넘는 문자는 거부해 배열 범위를 벗어나지 않는다.

## 실행

```powershell
cmake --preset vs2022-x64
cmake --build --preset vs2022-x64-debug
ctest --preset vs2022-x64-debug
```

VS Code에서는 저장소 루트를 연 채 CMake Tools의 Configure·Build·Test 버튼을 사용한다. `.vscode/settings.json`이 이 폴더와 `vs2022-x64` Preset을 자동으로 선택한다.

## 현재 상태

- 구현: 완료
- ARM GCC 문법·경고 검사: 완료 (`-Wall -Wextra -Wpedantic -Werror`)
- 호스트 CMake 빌드·CTest: 완료 (MSVC Build Tools, 1/1 통과)
- STM32 보드 검증: 해당 없음 — 다음 단계에서 UART DMA 코드와 연결
