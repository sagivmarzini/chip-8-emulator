#include <stdio.h>
#include <SDL3/SDL.h>

#include "chip8.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 320
#define PIXEL_SIZE 10

int main(int argc, char **argv) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    if (!SDL_CreateWindowAndRenderer("CHIP-8 EMULATOR", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
        SDL_Log("Window/Renderer Creation Error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    Chip8 cpu;
    chip8_init(&cpu);

    if (!chip8_load_rom(&cpu, "../games/bowling.ch8")) {
        return 1;
    }

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            // Catch both press and release events
            else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
                bool is_pressed = (event.type == SDL_EVENT_KEY_DOWN);

                // Map standard modern QWERTY keys to classic CHIP-8 Hex Keypad layout
                switch (event.key.key) {
                    case SDLK_ESCAPE: running = false;
                        break;
                    case SDLK_1: cpu.keypad[0x1] = is_pressed;
                        break;
                    case SDLK_2: cpu.keypad[0x2] = is_pressed;
                        break;
                    case SDLK_3: cpu.keypad[0x3] = is_pressed;
                        break;
                    case SDLK_4: cpu.keypad[0xC] = is_pressed;
                        break;
                    case SDLK_Q: cpu.keypad[0x4] = is_pressed;
                        break;
                    case SDLK_W: cpu.keypad[0x5] = is_pressed;
                        break;
                    case SDLK_E: cpu.keypad[0x6] = is_pressed;
                        break;
                    case SDLK_R: cpu.keypad[0xD] = is_pressed;
                        break;
                    case SDLK_A: cpu.keypad[0x7] = is_pressed;
                        break;
                    case SDLK_S: cpu.keypad[0x8] = is_pressed;
                        break;
                    case SDLK_D: cpu.keypad[0x9] = is_pressed;
                        break;
                    case SDLK_F: cpu.keypad[0xE] = is_pressed;
                        break;
                    case SDLK_Z: cpu.keypad[0xA] = is_pressed;
                        break;
                    case SDLK_X: cpu.keypad[0x0] = is_pressed;
                        break;
                    case SDLK_C: cpu.keypad[0xB] = is_pressed;
                        break;
                    case SDLK_V: cpu.keypad[0xF] = is_pressed;
                        break;
                }
            }
        }

        // Run ~8 CPU cycles per frame to get target speed (~500Hz)
        for (int i = 0; i < 8; i++) {
            chip8_cycle(&cpu);
        }

        // Update CHIP-8 timers at 60Hz (once per SDL frame loop)
        chip8_update_timers(&cpu);

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw Loop
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            for (int y = 0; y < DISPLAY_HEIGHT; y++) {
                SDL_FRect cell = {
                    .x = x * PIXEL_SIZE,
                    .y = y * PIXEL_SIZE,
                    .w = PIXEL_SIZE,
                    .h = PIXEL_SIZE
                };

                if (cpu.display[x][y]) {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // Sync loop closer to 60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
