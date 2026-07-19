cmake_minimum_required(VERSION 4.3)
project(chip_8_emulator C)

set(CMAKE_C_STANDARD 23)

include_directories(src)

find_package(SDL3 REQUIRED)

add_executable(chip_8_emulator
        src/chip8.c
        src/chip8.h
        src/main.c)

target_link_libraries(chip_8_emulator PRIVATE SDL3::SDL3)