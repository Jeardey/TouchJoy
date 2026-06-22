cmake_minimum_required(VERSION 3.15)
project(TouchJoy C CXX)

set(CMAKE_C_STANDARD 99)

# Fetch ViGEmClient automatically for Xbox 360 controller emulation
include(FetchContent)
FetchContent_Declare(
  ViGEmClient
  GIT_REPOSITORY https://github.com/nefarius/ViGEmClient.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(ViGEmClient)

add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    GB_INI_MAX_SECTION_LENGTH=16
    GB_INI_MAX_NAME_LENGTH=16
)

# 1. Main Windowed Application
add_executable(TouchJoy WIN32
    src/gamepad.c
    src/gamepad_window.c
    src/libs.c
    src/main.c
    src/utils.c
)
# Link ViGEm, Windows APIs, and Shell32 (for the browser prompt)
target_link_libraries(TouchJoy PRIVATE ViGEmClient setupapi.lib winmm.lib shell32.lib)

# 2. Test Console Application
add_executable(TouchJoyTest
    src/gamepad.c
    src/gamepad_window.c
    src/libs.c
    src/main.c
    src/utils.c
)
target_compile_definitions(TouchJoyTest PRIVATE _TEST)
target_link_libraries(TouchJoyTest PRIVATE ViGEmClient setupapi.lib winmm.lib shell32.lib)