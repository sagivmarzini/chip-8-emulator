#include "chip8.h"

int main(int argc, char **argv) {
    Chip8 cpu;
    chip8_init(&cpu);

    if (!chip8_load_rom(&cpu, "../games/breakout.ch8")) {
        return 1;
    }

    // TODO: Read keyboard events
    // TODO: Handle CPU waiting for key

    chip8_cycle(&cpu);

    // TODO: Draw display to screen

    return 0;
}
