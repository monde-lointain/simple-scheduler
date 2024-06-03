cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/stm32_gcc.cmake)

project(simple-scheduler LANGUAGES C ASM)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

stm32_fetch_cmsis(F4)

find_package(CMSIS COMPONENTS STM32F407VG REQUIRED)

add_subdirectory(src)
