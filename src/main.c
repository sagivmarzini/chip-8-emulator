#include "chip_8.h"

const bool program_running = true;

int main(int argc, char **argv) {
    Chip8 cpu;
    chip8_init(&cpu);
    
    if (!chip8_load_rom(&cpu, "games/pong.ch8")) {
        return 1;
    }
    
    // Main loop: Fetch, Decode, Execute, Draw
    while (program_running) {
        handle_input(&cpu);
        chip8_cycle(&cpu);
        
        // if (requested_draw) {
            render_screen(&cpu);
        // }
        
        delay_to_match_target_fps();
    }
    
    return 0;
}