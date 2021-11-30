#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "nes.h"
#include "getopt.c"
#include "settings.h"

#include <assert.h>
#include "mapper0.h"
#include "debug.h"

int main(int argc, char** argv) {
    // Slight delay, for IO reasons.
    SDL_Delay(400);

    // Parse arguments.
    char* filename = NULL;
    int opt = -1;
    while ((opt = getopt(argc, argv, "f:h")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                break;
            case 'h':
                printf("  -f [arg_name]  --  Specifies rom filename as arg_name.\n");
                printf("  -h             --  Print command line arguments.\n");
                break;
        }
    }

    // Return early is no filename is set.
    if (filename == NULL) return -1;

    // Declare and load cartridge from file.
    struct Cartridge cartridge;
    if (load_cartridge_from_file(filename, &cartridge) > 0) return -1;

    // Declare and initialize Nes struct.
    struct Nes nes;
    init_nes(&nes, cartridge);

    // 12 Savestates (NOTE: These wont work on non-NROM games!").
    struct Nes* savestates[12] = { NULL };

    // Render
    uint8_t lookup[] = {
        84, 84, 84, 0, 30, 116, 8, 16, 144, 48, 0, 136, 68, 0, 100, 92, 0, 48, 84, 4, 0, 60, 24, 0, 32, 42, 0, 8, 58, 0, 0, 64, 0, 0, 60, 0, 0, 50, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        152, 150, 152, 8, 76, 196, 48, 50, 236, 92, 30, 228, 136, 20, 176, 160, 20, 100, 152, 34, 32, 120, 60, 0, 84, 90, 0, 40, 114, 0, 8, 124, 0, 0, 118, 40, 0, 102, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        236, 238, 236, 76, 154, 236, 120, 124, 236, 176, 98, 236, 228, 84, 236, 236, 88, 180, 236, 106, 100, 212, 136, 32, 160, 170, 0, 116, 196, 0, 76, 208, 32, 56, 204, 108, 56, 180, 204, 60, 60, 60, 0, 0, 0, 0, 0, 0, 
        236, 238, 236, 168, 204, 236, 188, 188, 236, 212, 178, 236, 236, 174, 236, 236, 174, 212, 236, 180, 176, 228, 196, 144, 204, 210, 120, 180, 222, 120, 168, 226, 144, 152, 226, 180, 160, 214, 228, 160, 162, 160, 0, 0, 0, 0, 0, 0
    };

    // Declare and load settings from file. A new settings is made if settings.conf is not found.
    struct Settings settings;
    settings_load_from_file(&settings, "settings.conf");

    // Initialize SDL vdieo.
    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);

    // Create main SDL window.
    SDL_Window* window = SDL_CreateWindow("NES Emu", 1920/2, 1080/2, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    assert(window);

    // Declare space for screen output. 
    uint8_t screen_pixels[SCREEN_WIDTH * SCREEN_HEIGHT * 3];
    // Declare space for PPU output.
    uint8_t nes_pixels[SCREEN_WIDTH * SCREEN_HEIGHT];

    // The main game loop. Runs at 60 Hz.
    int vblanks = 0;
    uint8_t next = 0;
    while (1) {
        // For sleep timing.
        int start = SDL_GetTicks();

        // Step the CPU and PPU components. This is the heart of the emulation.
        int cycle = step_cpu(&nes);
        int vblank = step_ppu(&nes, 3 * cycle, nes_pixels); // 1 CPU clock = 3 PPU clocks

        // Once per frame logic.
        if (vblank) {
            vblanks += 1;

            // Sprite drawing is currently not cycle accurate
            draw_oam_sprites(&nes, nes_pixels);

            // Poll window events.
            int keys = 0;
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_WINDOWEVENT) {
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_MOVED: {
                            nlog("moved");
                        } break;
                        case SDL_WINDOWEVENT_CLOSE: {
                            goto exit;
                        } break;
                    }
                }
            }

            // Check keyboard input, pack and record it in Nes object.
            const uint8_t* keyboard = SDL_GetKeyboardState(&keys);
            uint8_t state = 0;
            state |= keyboard[settings.right] << 7;
            state |= keyboard[settings.left] << 6;
            state |= keyboard[settings.down] << 5;
            state |= keyboard[settings.up] << 4;
            state |= keyboard[settings.start] << 3;
            state |= keyboard[settings.select] << 2;
            state |= keyboard[settings.b] << 1;
            state |= keyboard[settings.a] << 0;
            nes.input1 = state;

            // Savestate logic.
            char fcodes[12];
            fcodes[0] = keyboard[SDL_SCANCODE_F1];
            fcodes[1] = keyboard[SDL_SCANCODE_F2];
            fcodes[2] = keyboard[SDL_SCANCODE_F3];
            fcodes[3] = keyboard[SDL_SCANCODE_F4];
            fcodes[4] = keyboard[SDL_SCANCODE_F5];
            fcodes[5] = keyboard[SDL_SCANCODE_F6];
            fcodes[6] = keyboard[SDL_SCANCODE_F7];
            fcodes[7] = keyboard[SDL_SCANCODE_F8];
            fcodes[8] = keyboard[SDL_SCANCODE_F9];
            fcodes[9] = keyboard[SDL_SCANCODE_F10];
            fcodes[10] = keyboard[SDL_SCANCODE_F11];
            fcodes[11] = keyboard[SDL_SCANCODE_F12];
            for (int i = 0; i < 12; i++) {
                if (fcodes[i]) {
                    if (keyboard[SDL_SCANCODE_LSHIFT]) {
                        if (savestates[i] == 0) savestates[i] = malloc(sizeof(struct Nes));
                        *savestates[i] = nes;
                    } else {
                        if (savestates[i] != 0) nes = *savestates[i];
                    }
                }
            }

            // Convert NES PPU output to RGB for screen render output.
            for (int y = 0; y < SCREEN_HEIGHT; y++) {
                for (int x = 0; x < SCREEN_WIDTH; x++) {
                    uint32_t dst_i = x + y * SCREEN_WIDTH;
                    uint32_t src_i = x + y * SCREEN_WIDTH;
                    screen_pixels[dst_i * 3 + 0] = lookup[nes_pixels[src_i] * 3 + 0];
                    screen_pixels[dst_i * 3 + 1] = lookup[nes_pixels[src_i] * 3 + 1];
                    screen_pixels[dst_i * 3 + 2] = lookup[nes_pixels[src_i] * 3 + 2];
                }
            }     

            // Render RGB pixels to screen.
            SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(screen_pixels, 256, 240, 24, 256 * 3, 0x0000FF, 0x00FF00, 0xFF0000, 0);
            assert(surface);
            SDL_BlitSurface(surface, 0, SDL_GetWindowSurface(window), 0);
            SDL_UpdateWindowSurface(window); 
            SDL_FreeSurface(surface);

            // Sleep for (16ms - time_taken).
            int end = SDL_GetTicks();
            if (end - start < 16) {
                SDL_Delay(16 - (end - start));
            }
        };
    }

exit:
    // Write settings to settings.conf.
    settings_write_to_file(&settings, "settings.conf");

    // Free main SDL window.
    SDL_DestroyWindow(window);

    // Free Nes object.
    free_nes(&nes);
}