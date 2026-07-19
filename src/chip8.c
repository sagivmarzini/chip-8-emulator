#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Font set for rendering
static const uint8_t FONTSET[NUM_FONT_DIGITS][DIGIT_SIZE] = {
    {0xF0, 0x90, 0x90, 0x90, 0xF0}, // 0
    {0x20, 0x60, 0x20, 0x20, 0x70}, // 1
    {0xF0, 0x10, 0xF0, 0x80, 0xF0}, // 2
    {0xF0, 0x10, 0xF0, 0x10, 0xF0}, // 3
    {0x90, 0x90, 0xF0, 0x10, 0x10}, // 4
    {0xF0, 0x80, 0xF0, 0x10, 0xF0}, // 5
    {0xF0, 0x80, 0xF0, 0x90, 0xF0}, // 6
    {0xF0, 0x10, 0x20, 0x40, 0x40}, // 7
    {0xF0, 0x90, 0xF0, 0x90, 0xF0}, // 8
    {0xF0, 0x90, 0xF0, 0x10, 0xF0}, // 9
    {0xF0, 0x90, 0xF0, 0x90, 0x90}, // A
    {0xE0, 0x90, 0xE0, 0x90, 0xE0}, // B
    {0xF0, 0x80, 0x80, 0x80, 0xF0}, // C
    {0xE0, 0x90, 0x90, 0x90, 0xE0}, // D
    {0xF0, 0x80, 0xF0, 0x80, 0xF0}, // E
    {0xF0, 0x80, 0xF0, 0x80, 0x80}, // F
};

void chip8_init(Chip8 *cpu) {
    memset(cpu, 0, sizeof(Chip8));

    cpu->pc = 0x200; // CHIP-8 programs start at memory address 0x200

    memcpy(cpu->ram + FONT_STORE_OFFSET, FONTSET, sizeof(FONTSET));

    srand(time(NULL));
}

bool chip8_load_rom(Chip8 *cpu, const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    long file_size = 0;

    if (!file) {
        perror("Error opening ROM file");
        return false;
    }

    // find file size and load program
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    printf("ROM size: %ld\n", file_size);
    rewind(file);

    // return error if file is too large or cannot be found
    if (file_size > MAX_FILE_SIZE) {
        printf("File is too large to load.\n");
        return false;
    }

    // read program into memory
    fread(cpu->ram + 0x200, 1, file_size, file);
    return true;
}

void chip8_cycle(Chip8 *cpu) {
    if (cpu->waiting_for_key) {
        bool key_pressed = false;
        for (int i = 0; i < 16; i++) {
            if (cpu->keypad[i]) {
                cpu->V[cpu->waiting_register] = i;
                cpu->waiting_for_key = false;
                key_pressed = true;
                break;
            }
        }
        // If no key was pressed, exit early and don't execute the next cycle
        if (!key_pressed) {
            return;
        }
    }

    // Read the first byte, push it off to the right and add the second byte
    const uint16_t instruction = (cpu->ram[cpu->pc] << 8) | cpu->ram[cpu->pc + 1];
    cpu->pc += 2;

    const uint16_t op = (instruction & 0xF000);
    const uint8_t x = (instruction & 0x0F00) >> 8;
    const uint8_t y = (instruction & 0x00F0) >> 4;
    const uint8_t n = (instruction & 0x000F);
    const uint8_t nn = (instruction & 0x00FF);
    const uint16_t nnn = (instruction & 0x0FFF);

    uint8_t *V = cpu->V;

    switch (op) {
        case 0x0000:
            switch (nn) {
                case 0xE0: // Clear display
                    memset(cpu->display, 0, sizeof(cpu->display));
                    break;
                case 0xEE: // Return from subroutine
                    cpu->pc = cpu->stack[cpu->sp];
                    cpu->sp--;
                    break;
            }
            break;
        case 0x1000: // Jump to location nnn
            cpu->pc = nnn;
            break;
        case 0x2000: // Call function at nnn
            cpu->sp++;
            cpu->stack[cpu->sp] = cpu->pc;
            cpu->pc = nnn;
            break;
        case 0x3000: // Skip next instruction if Vx = nn
            if (V[x] == nn)
                cpu->pc += 2;
            break;
        case 0x4000: // Skip next instruction if Vx != nn
            if (V[x] != nn)
                cpu->pc += 2;
            break;
        case 0x5000: // Skip next instruction if Vx = Vy
            if (V[x] == V[y])
                cpu->pc += 2;
            break;
        case 0x6000: // Set Vx = nn
            V[x] = nn;
            break;
        case 0x7000: // Adds NN to Vx (store in Vx)
            V[x] += nn;
            break;
        case 0x8000: // Math instructions
            switch (n) {
                case 0x0:
                    V[x] = V[y];
                    break;
                case 0x1:
                    V[x] |= V[y];
                    break;
                case 0x2:
                    V[x] &= V[y];
                    break;
                case 0x3:
                    V[x] ^= V[y];
                    break;
                case 0x4:
                    V[0xF] = (V[x] + V[y]) > UINT8_MAX;
                    V[x] += V[y];
                    break;
                case 0x5:
                    V[0xF] = V[x] >= V[y];
                    V[x] -= V[y];
                    break;
                case 0x6: // Shift right
                    V[0xF] = V[x] & 0x1;
                    V[x] >>= 1;
                    break;
                case 0x7:
                    V[0xF] = V[y] >= V[x];
                    V[x] = V[y] - V[x];
                    break;
                case 0xE: // Shift left
                    V[0xF] = (V[x] & 0x80) >> 7;
                    V[x] <<= 1;
                    break;
            }
            break;
        case 0x9000: // Skip next instruction if Vx != Vy
            if (V[x] != V[y])
                cpu->pc += 2;
            break;
        case 0xA000:
            cpu->I = nnn;
            break;
        case 0xB000:
            cpu->pc = nnn + V[0];
            break;
        case 0xC000:
            V[x] = rand() & nn;
            break;
        case 0xD000: // Display n-byte sprite starting at memory location I at (Vx, Vy)
            V[0xF] = 0;

            // Loop through the bits of the sprite
            for (uint8_t row = 0; row < n; row++) {
                for (uint8_t col = 0; col < 8; col++) {
                    uint8_t screen_x = (V[x] + col) % 64;
                    uint8_t screen_y = (V[y] + row) % 32;

                    uint8_t sprite_pixel = (cpu->ram[cpu->I + row] & (0b10000000 >> col)) ? 1 : 0;
                    uint8_t screen_pixel = cpu->display[screen_x][screen_y];

                    if (sprite_pixel && screen_pixel) {
                        V[0xF] = 1;
                    }

                    cpu->display[screen_x][screen_y] ^= sprite_pixel;
                }
            }

            break;
        case 0xE000: // Keypad input
            switch (nn) {
                case 0x9E: // Skip next instruction if key with the value of Vx is pressed.
                    if (cpu->keypad[V[x]] == true)
                        cpu->pc += 2;
                    break;
                case 0xA1: // Skip next instruction if key with the value of Vx is not pressed.
                    if (cpu->keypad[V[x]] == false)
                        cpu->pc += 2;
                    break;
            }
            break;
        case 0xF000:
            switch (nn) {
                case 0x07:
                    V[x] = cpu->delay_timer;
                    break;
                case 0x0A: // Wait for a key press, store the value of the key in Vx
                    cpu->waiting_for_key = true;
                    cpu->waiting_register = x;
                    break;
                case 0x15:
                    cpu->delay_timer = V[x];
                    break;
                case 0x18:
                    cpu->sound_timer = V[x];
                    break;
                case 0x1E:
                    cpu->I += V[x];
                    break;
                case 0x29: // Set I = location of sprite for digit Vx
                    cpu->I = FONT_STORE_OFFSET + V[x] * DIGIT_SIZE;
                    break;
                case 0x33: // Store BCD representation of Vx in memory locations I, I+1, and I+2
                    const uint8_t num = V[x];
                    cpu->ram[cpu->I] = (num / 100) % 10;
                    cpu->ram[cpu->I + 1] = (num / 10) % 10;
                    cpu->ram[cpu->I + 2] = (num) % 10;
                    break;
                case 0x55:
                    for (int i = 0; i <= x; i++) {
                        cpu->ram[cpu->I + i] = V[i];
                    }
                    break;
                case 0x65:
                    for (int i = 0; i <= x; i++) {
                        V[i] = cpu->ram[cpu->I + i];
                    }
                    break;
            }
            break;
    }
}

void chip8_update_timers(Chip8 *cpu) {
    if (cpu->delay_timer > 0) cpu->delay_timer--;
    if (cpu->sound_timer > 0) cpu->sound_timer--;
}
