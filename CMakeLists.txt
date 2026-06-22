cmake_minimum_required(VERSION 3.15)
project(TouchJoy C)

# Force standard C
set(CMAKE_C_STANDARD 99)

# Global Defines (Copied from the old genie.lua)
add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    GB_INI_MAX_SECTION_LENGTH=16
    GB_INI_MAX_NAME_LENGTH=16
)

# 1. Main Windowed Application (WIN32 hides the console window)
add_executable(TouchJoy WIN32
    src/gamepad.c
    src/gamepad_window.c
    src/libs.c
    src/main.c
    src/utils.c
)

# 2. Test Console Application
add_executable(TouchJoyTest
    src/gamepad.c
    src/gamepad_window.c
    src/libs.c
    src/main.c
    src/utils.c
)
# Add the _TEST flag only to the test build
target_compile_definitions(TouchJoyTest PRIVATE _TEST)