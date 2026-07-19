#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <SDL3/SDL.h>

#include "chip8.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 320
#define PIXEL_SIZE 10

#define AUDIO_FREQ 44100
#define BEEP_HZ 440 // Tone pitch (A4 note)
#define MAX_GAMES 32
#define MAX_PATH_LEN 512

// Scans the games directory and returns the path of the user's selected game
bool select_game(char *out_path) {
    const char *dir_path = "../games";
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Error opening games directory");
        printf("Make sure the execution path has access to '%s'\n", dir_path);
        return false;
    }

    struct dirent *entry;
    char games[MAX_GAMES][256];
    int game_count = 0;

    printf("\n=== CHIP-8 ROM SELECTOR ===\n");
    while ((entry = readdir(dir)) != NULL && game_count < MAX_GAMES) {
        // Only list files ending with .ch8
        if (strstr(entry->d_name, ".ch8") != NULL) {
            strncpy(games[game_count], entry->d_name, sizeof(games[game_count]) - 1);
            printf("[%d] %s\n", game_count + 1, games[game_count]);
            game_count++;
        }
    }
    closedir(dir);

    if (game_count == 0) {
        printf("No .ch8 files found in '%s'.\n", dir_path);
        return false;
    }

    int choice = 0;
    while (choice < 1 || choice > game_count) {
        printf("\nSelect a game number (1-%d): ", game_count);
        if (scanf("%d", &choice) != 1) {
            // Clear buffer on invalid input
            while (getchar() != '\n');
            printf("Invalid input. Please enter a number.\n");
        }
    }

    // Construct full relative path to the ROM
    snprintf(out_path, MAX_PATH_LEN, "%s/%s", dir_path, games[choice - 1]);
    printf("Loading: %s\n\n", out_path);
    return true;
}

int main(int argc, char **argv) {
    char selected_rom[MAX_PATH_LEN];
    if (!select_game(selected_rom)) {
        return 1;
    }

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
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        SDL_Log("SDL Audio init failed: %s", SDL_GetError());
    }

    SDL_AudioSpec spec = {
        .format = SDL_AUDIO_S16,
        .channels = 1,
        .freq = 44100
    };

    // Open audio device and create stream in one shot
    SDL_AudioStream *audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);

    if (!audio_stream) {
        SDL_Log("Failed to create audio stream: %s", SDL_GetError());
    }

    int16_t beep_buffer[AUDIO_FREQ];
    for (int i = 0; i < AUDIO_FREQ; i++) {
        // Generate a simple square wave alternating between high and low volume
        beep_buffer[i] = ((i * BEEP_HZ / AUDIO_FREQ) % 2 == 0) ? 3000 : -3000;
    }

    Chip8 cpu;
    chip8_init(&cpu);

    if (!chip8_load_rom(&cpu, selected_rom)) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
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
        if (audio_stream) {
            if (cpu.sound_timer > 0) {
                // If the stream is running low on audio data, queue up more beep samples
                if (SDL_GetAudioStreamQueued(audio_stream) < 2000) {
                    SDL_PutAudioStreamData(audio_stream, beep_buffer, 735 * sizeof(int16_t));
                }
                SDL_ResumeAudioStreamDevice(audio_stream);
            } else {
                // Stop playing and clear out any leftover audio data instantly
            pragma_pause:
                SDL_PauseAudioStreamDevice(audio_stream);
                SDL_ClearAudioStream(audio_stream);
            }
        }

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
    if (audio_stream) {
        SDL_DestroyAudioStream(audio_stream);
    }
    SDL_Quit();
    return 0;
}
