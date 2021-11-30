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
    //
    SDL_Delay(400);

    // arg parsing
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

    if (filename == NULL) return -1;

    // load ROM
    struct Cartridge cartridge;
    if (load_cartridge_from_file(filename, &cartridge) > 0) return -1;

    // load nes
    struct Nes nes;
    init_nes(&nes, cartridge);

    // Render
    uint8_t lookup[] = {
        84, 84, 84, 0, 30, 116, 8, 16, 144, 48, 0, 136, 68, 0, 100, 92, 0, 48, 84, 4, 0, 60, 24, 0, 32, 42, 0, 8, 58, 0, 0, 64, 0, 0, 60, 0, 0, 50, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        152, 150, 152, 8, 76, 196, 48, 50, 236, 92, 30, 228, 136, 20, 176, 160, 20, 100, 152, 34, 32, 120, 60, 0, 84, 90, 0, 40, 114, 0, 8, 124, 0, 0, 118, 40, 0, 102, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        236, 238, 236, 76, 154, 236, 120, 124, 236, 176, 98, 236, 228, 84, 236, 236, 88, 180, 236, 106, 100, 212, 136, 32, 160, 170, 0, 116, 196, 0, 76, 208, 32, 56, 204, 108, 56, 180, 204, 60, 60, 60, 0, 0, 0, 0, 0, 0, 
        236, 238, 236, 168, 204, 236, 188, 188, 236, 212, 178, 236, 236, 174, 236, 236, 174, 212, 236, 180, 176, 228, 196, 144, 204, 210, 120, 180, 222, 120, 168, 226, 144, 152, 226, 180, 160, 214, 228, 160, 162, 160, 0, 0, 0, 0, 0, 0
    };

    struct Settings settings;
    settings_load_from_file(&settings, "settings.conf");

    // init SDL video
    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);

    // create windows
    SDL_Window* window = SDL_CreateWindow("NES Emu", 1920/2, 1080/2, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    assert(window);
    SDL_Window* nametable_window = SDL_CreateWindow("NES Emu: Nametable", 2 * SCREEN_WIDTH, 2 * SCREEN_HEIGHT, 2 * SCREEN_WIDTH, 2 * SCREEN_HEIGHT, 0);
    assert(nametable_window);

    // allocate pixel space
    uint8_t* screen_pixels = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 3);
    uint8_t* nes_pixels = malloc(SCREEN_WIDTH * SCREEN_HEIGHT);

    // run n vblanks of rom
    int acc_cycle = 7;
    int vblanks = 0;

    uint8_t next = 0;
    while (1) {
        int start = SDL_GetTicks();
        int cycle = step_cpu(&nes);

        if (step_ppu(&nes, 3 * cycle, nes_pixels)) {
            vblanks += 1;

            // debug render nametable
            uint8_t nametable_pixels[SCREEN_WIDTH * SCREEN_HEIGHT * 4];
            draw_ppu_nametables(&nes, nametable_pixels);
            uint8_t nametable_rgb[SCREEN_WIDTH * SCREEN_HEIGHT * 4 * 3];
            for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 4; i++) {
                nametable_rgb[3 * i + 0] = lookup[3 * nametable_pixels[i] + 0];
                nametable_rgb[3 * i + 1] = lookup[3 * nametable_pixels[i] + 1];
                nametable_rgb[3 * i + 2] = lookup[3 * nametable_pixels[i] + 2];
            }
            SDL_Surface* temp = SDL_CreateRGBSurfaceFrom(nametable_rgb, 2 * SCREEN_WIDTH, 2 * SCREEN_HEIGHT, 24, 3 * 2 * SCREEN_WIDTH, 0x0000FF, 0x00FF00, 0xFF0000, 0);
            SDL_BlitSurface(temp, 0, SDL_GetWindowSurface(nametable_window), 0);
            SDL_UpdateWindowSurface(nametable_window); 
            SDL_FreeSurface(temp);

            // TEMP
            draw_oam_sprites(&nes, nes_pixels);

            // keyboard
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

            // convert NES pixel to RGB pixels
            for (int y = 0; y < SCREEN_HEIGHT; y++) {
                for (int x = 0; x < SCREEN_WIDTH; x++) {
                    uint32_t dst_i = x + y * SCREEN_WIDTH;
                    uint32_t src_i = x + y * SCREEN_WIDTH;
                    screen_pixels[dst_i * 3 + 0] = lookup[nes_pixels[src_i] * 3 + 0];
                    screen_pixels[dst_i * 3 + 1] = lookup[nes_pixels[src_i] * 3 + 1];
                    screen_pixels[dst_i * 3 + 2] = lookup[nes_pixels[src_i] * 3 + 2];
                }
            }     

            // blit and render (vsync?)
            SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(screen_pixels, 256, 240, 24, 256 * 3, 0x0000FF, 0x00FF00, 0xFF0000, 0);
            assert(surface);
            SDL_BlitSurface(surface, 0, SDL_GetWindowSurface(window), 0);
            SDL_UpdateWindowSurface(window); 
            SDL_FreeSurface(surface);

            int end = SDL_GetTicks();
            if (end - start < 16) {
                SDL_Delay(16 - (end - start));
            }
        };
        acc_cycle += cycle;
    }

exit:

    //
    settings_write_to_file(&settings, "settings.conf");

    // free render
    SDL_DestroyWindow(window);
    SDL_DestroyWindow(nametable_window);
    free(screen_pixels);
    free(nes_pixels);

    // free nes
    free_nes(&nes);
}