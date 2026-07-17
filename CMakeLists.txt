cmake_minimum_required(VERSION 4.3)
project(chip_8_emulator C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(chip_8_emulator
        src/chip8.c
        src/chip8.h
        src/main.c)
