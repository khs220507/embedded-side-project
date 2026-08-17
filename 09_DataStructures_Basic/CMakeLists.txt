cmake_minimum_required(VERSION 3.20)

project(DataStructuresBasic LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(data_structures_basic
    src/main.c
)

if(MSVC)
    target_compile_options(data_structures_basic PRIVATE /W4 /WX)
else()
    target_compile_options(data_structures_basic PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()
add_test(NAME data_structures_basic_tests COMMAND data_structures_basic)
