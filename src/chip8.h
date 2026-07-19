#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

#define SYSTEM_MEMORY	4096
#define NUM_REGISTERS	16
#define STACK_SIZE		16
#define SCREEN_WIDTH	64
#define SCREEN_HEIGHT	32
#define NUM_KEYS		16
#define MAX_FILE_SIZE	0xE00
#define SPRITE_WIDTH	8
#define FONT_STORE_OFFSET 0
#define NUM_FONT_DIGITS 16
#define DIGIT_SIZE	5
#define MAX_SPRITE_HEIGHT 15

typedef struct {
    uint8_t ram[SYSTEM_MEMORY]; // 4KB of memory
    uint8_t V[NUM_REGISTERS]; // General-purpose registers V0-VF
    uint16_t I; // Index register
    uint16_t pc; // Program counter
    uint16_t stack[STACK_SIZE]; // Call stack
    uint8_t sp; // Stack pointer

    uint8_t delay_timer;
    uint8_t sound_timer;

    uint8_t display[SCREEN_WIDTH][SCREEN_HEIGHT]; // Monochrome screen state (1 for on, 0 for off)
    bool keypad[NUM_KEYS]; // Key state (true = pressed)

    // For when waiting for a key press
    bool waiting_for_key;
    uint8_t waiting_register;
} Chip8;

void chip8_init(Chip8 *cpu);

bool chip8_load_rom(Chip8 *cpu, const char *filepath);

void chip8_cycle(Chip8 *cpu);

#endif
