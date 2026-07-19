#include "chip8.h"

int main(int argc, char **argv) {
    Chip8 cpu;
    chip8_init(&cpu);

    if (!chip8_load_rom(&cpu, "../games/breakout.ch8")) {
        return 1;
    }

    chip8_cycle(&cpu);

    return 0;
}
