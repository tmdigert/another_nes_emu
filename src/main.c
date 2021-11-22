#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <string.h>

#include "nes.h"
#include "error.h"

// opcode ID to name lookup table
char* lookup_opcode(uint8_t op) {
    switch (op) {
        case 0x69: case 0x65: case 0x75: case 0x6D: case 0x7D: case 0x79: case 0x61: case 0x71: return "adc";
        case 0x29: case 0x25: case 0x35: case 0x2D: case 0x3D: case 0x39: case 0x21: case 0x31: return "and";
        case 0x0A: case 0x06: case 0x16: case 0x0E: case 0x1E: return "asl";
        case 0x24: case 0x2C: return "bit";
        case 0x10: return "bpl";
        case 0x30: return "bmi";
        case 0x50: return "bvc";
        case 0x70: return "bvs";
        case 0x90: return "bcc";
        case 0xB0: return "bcs";
        case 0xD0: return "bne";
        case 0xF0: return "beq";
        case 0x00: return "brk";
        case 0xC9: case 0xC5:  case 0xD5:  case 0xCD:  case 0xDD:  case 0xD9:  case 0xC1:  case 0xD1: return "cmp";
        case 0xE0: case 0xE4: case 0xEC: return "cpx";
        case 0xC0: case 0xC4: case 0xCC: return "cpy";
        case 0xC6: case 0xD6: case 0xCE: case 0xDE: return "dec";
        case 0x49: case 0x45: case 0x55: case 0x4D: case 0x5D: case 0x59: case 0x41: case 0x51: return "eor";
        case 0x18: return "clc";
        case 0x38: return "sec";
        case 0x58: return "cli";
        case 0x78: return "sei";
        case 0xB8: return "clv";
        case 0xD8: return "cld";
        case 0xF8: return "sed";
        case 0xE6: case 0xF6: case 0xEE: case 0xFE: return "inc";
        case 0x4C: case 0x6C: return "jmp";
        case 0x20: return "jsr";
        case 0xA9: case 0xA5: case 0xB5: case 0xAD: case 0xBD: case 0xB9: case 0xA1: case 0xB1: return "lda";
        case 0xA2: case 0xA6: case 0xB6: case 0xAE: case 0xBE: return "ldx";
        case 0xA0: case 0xA4: case 0xB4: case 0xAC: case 0xBC: return "ldy";
        case 0x4A: case 0x46: case 0x56: case 0x4E: case 0x5E: return "lsr";
        case 0xEA: case 0x1A: case 0x3A: case 0x5A: case 0x7A: case 0xDA: case 0xFA: case 0x04: case 0x44: case 0x64: case 0x0C: case 0x14: case 0x34: case 0x54: case 0x74: case 0xD4: case 0xF4: case 0x80: case 0x1C: case 0x3C: case 0x5C: case 0x7C: case 0xDC: case 0xFC: return "nop";
        case 0x09: case 0x05: case 0x15: case 0x0D: case 0x1D: case 0x19: case 0x01: case 0x11: return "ora";
        case 0xAA: return "tax";
        case 0x8A: return "txa";
        case 0xCA: return "dex";
        case 0xE8: return "inx";
        case 0xA8: return "tay";
        case 0x98: return "tya";
        case 0x88: return "dey";
        case 0xC8: return "iny";
        case 0x2A: case 0x26: case 0x36: case 0x2E: case 0x3E: return "rol";
        case 0x6A: case 0x66: case 0x76: case 0x6E: case 0x7E: return "ror";
        case 0x40: return "rti";
        case 0x60: return "rts";
        case 0xE9: case 0xE5: case 0xF5: case 0xED: case 0xFD: case 0xF9: case 0xE1: case 0xF1: return "sbc";
        case 0x85: case 0x95: case 0x8D: case 0x9D: case 0x99: case 0x81: case 0x91: return "sta";
        case 0x9A: return "txs";
        case 0xBA: return "tsx";
        case 0x48: return "pha";
        case 0x68: return "pla";
        case 0x08: return "php";
        case 0x28: return "plp";
        case 0x86: case 0x96: case 0x8E: return "stx"; 
        case 0x84: case 0x94: case 0x8C: return "sty";
        default: return "inv";
    }
}

#include <assert.h>

void draw_sprites(struct Nes* nes, uint8_t* pixels) {
    // constants
    uint16_t nametable_width = 32; // in tiles
    uint16_t nametable_height = 30; // in tiles
    uint16_t tile_size = 8; // in pixels
    uint16_t sprite_size = 8; // in pixels

    uint8_t spr_palette[4][4];
    spr_palette[0][0] = ppu_bus_read(nes, 0x3F10);
    spr_palette[0][1] = ppu_bus_read(nes, 0x3F11);
    spr_palette[0][2] = ppu_bus_read(nes, 0x3F12);
    spr_palette[0][3] = ppu_bus_read(nes, 0x3F13);
    spr_palette[1][0] = ppu_bus_read(nes, 0x3F14);
    spr_palette[1][1] = ppu_bus_read(nes, 0x3F15);
    spr_palette[1][2] = ppu_bus_read(nes, 0x3F16);
    spr_palette[1][3] = ppu_bus_read(nes, 0x3F17);
    spr_palette[2][0] = ppu_bus_read(nes, 0x3F18);
    spr_palette[2][1] = ppu_bus_read(nes, 0x3F19);
    spr_palette[2][2] = ppu_bus_read(nes, 0x3F1A);
    spr_palette[2][3] = ppu_bus_read(nes, 0x3F1B);
    spr_palette[3][0] = ppu_bus_read(nes, 0x3F1C);
    spr_palette[3][1] = ppu_bus_read(nes, 0x3F1D);
    spr_palette[3][2] = ppu_bus_read(nes, 0x3F1E);
    spr_palette[3][3] = ppu_bus_read(nes, 0x3F1F);

    // sprites
    for (uint8_t sprite = 0; sprite < 64; sprite++) {
        // sprite y positions
        uint8_t byte0 = nes->oam[sprite * 4 + 0];
        // sprite index information
        uint8_t byte1 = nes->oam[sprite * 4 + 1];
        // sprite attribute information
        uint8_t byte2 = nes->oam[sprite * 4 + 2];
        // sprite x position
        uint8_t byte3 = nes->oam[sprite * 4 + 3];

        if (byte0 >= 0xEF || byte3 >= 0xF9) {
            continue;
        }

        //
        uint8_t palette_id = byte2 & 0b11;

        uint16_t pattern_table_base = (byte1) << 4;
        uint8_t mode = (nes->ppuctrl & 0b00100000) > 1;
        if (mode == 0) {
            pattern_table_base |= nes->ppuctrl << 9 & 0b1000000000000;
        } else {
            assert(0);
            pattern_table_base |= (byte0 & 0x1) * 0x1000;
        }
        assert((nes->ppuctrl & 0b100000) == 0);
        for (uint8_t spr = 0; spr < mode + 1; spr++) {
            for (uint8_t bit_plane = 0; bit_plane < sprite_size; bit_plane++) {
                //
                uint8_t low_bits = ppu_bus_read(nes, (pattern_table_base + spr) | bit_plane); // low bits for 8 pixels
                uint8_t high_bits = ppu_bus_read(nes, (pattern_table_base + spr) | sprite_size | bit_plane); // high bits for 8 pixels
    
                //
                for (uint8_t pixel_index = 0; pixel_index < sprite_size; pixel_index++) {
                    //
                    uint8_t pixel = high_bits % 2 << 1 | low_bits % 2;
                    low_bits >>= 1;
                    high_bits >>= 1;
                    if (pixel == 0) continue;
    
                    //
                    uint16_t pixel_x = byte3 + ((byte2 & 0x40) > 0 ? pixel_index : sprite_size - 1 - pixel_index);
                    uint16_t pixel_y = 1 + byte0 + spr * 8 + ((byte2 & 0x80) > 0 ? sprite_size - 1 - bit_plane : bit_plane); // this ternary possibly wrong
                    if (pixel_x >= 256 || pixel_y >= 240) continue;
                    uint32_t index = pixel_x + pixel_y * nametable_width * tile_size;
    
                    //

                    pixels[index] = spr_palette[palette_id][pixel];
                }
            }
        }
    }
}

#include <assert.h>
#include "mapper0.h"

int main(int argc, char* argv[]) {
    // load ROM
    struct Cartridge cartridge;
    if (load_cartridge_from_file("smb.nes", &cartridge) > 0) return -1;

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
    uint32_t nes_width = 256;
    uint32_t nes_height = 240;

    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
    SDL_Window* window = SDL_CreateWindow("NES Emu", 1920/2, 1080/2, 256, 240, 0);
    assert(window);
    SDL_Surface* screen = SDL_GetWindowSurface(window);
    assert(screen);

    uint8_t* screen_pixels = malloc(nes_width * nes_height * 3);
    uint8_t* nes_pixels = malloc(nes_width * nes_height);

    // run n vblanks of rom
    int acc_cycle = 7;
    int vblanks = 0;

    uint8_t next = 0;
    while (1) {
        //uint8_t next = cpu_bus_read(&nes, nes.pc);
        //nlog("%6i  %04X  %02X    %s                             A:%02X X:%02X Y:%02X P:%02X SP:%02X             CYC:%i\n", i, nes.pc, next, lookup_opcode(next), nes.acc, nes.x, nes.y, nes.status, nes.sp, acc_cycle);
            /*if (nes.pc >= 0xF11E && nes.pc <= 0xF139) {
                uint8_t next = cpu_bus_read(&nes, nes.pc);
                nlog("%04X  %02X    %s                             A:%02X X:%02X Y:%02X P:%02X SP:%02X", nes.pc, next, lookup_opcode(next), nes.acc, nes.x, nes.y, nes.status, nes.sp);
            } */        


        /*if ((nes.pc >= 0xD5B7 && nes.pc <= 0xD5DF) && nes.pc != 0xD5C4 && nes.pc != 0xD5C6) {
            uint8_t next = cpu_bus_read(&nes, nes.pc);
            nlog("$%04X: %s [A:$%02X, X:$%02X, Y:$%02X]", nes.pc, lookup_opcode(next), nes.acc, nes.x, nes.y);
        }*/

        int start = SDL_GetTicks();
        int cycle = step_cpu(&nes);
        if (step_ppu(&nes, 3 * cycle, nes_pixels)) {
            draw_sprites(&nes, nes_pixels);
            vblanks += 1;

            // keyboard
            int keys = 0;
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) goto exit;
                if (event.type == SDL_WINDOWEVENT) {
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_MOVED: {
                            nlog("moved");
                        } break;
                    }
                }
            }
            const uint8_t* keyboard = SDL_GetKeyboardState(&keys);
            uint8_t state = 0;
            state |= keyboard[SDL_SCANCODE_RIGHT] << 7;
            state |= keyboard[SDL_SCANCODE_LEFT] << 6;
            state |= keyboard[SDL_SCANCODE_DOWN] << 5;
            state |= keyboard[SDL_SCANCODE_UP] << 4;
            state |= keyboard[SDL_SCANCODE_RETURN] << 3;
            state |= keyboard[SDL_SCANCODE_RSHIFT] << 2;
            state |= keyboard[SDL_SCANCODE_X] << 1;
            state |= keyboard[SDL_SCANCODE_Z] << 0;
            nes.input1 = state;

            // render 
            //fill_nametable(&nes, nes_pixels);

            // convert NES pixel to RGB pixels
            for (int y = 0; y < nes_height; y++) {
                for (int x = 0; x < nes_width; x++) {
                    uint32_t dst_i = x + y * nes_width;
                    uint32_t src_i = x + y * nes_width;
                    screen_pixels[dst_i * 3 + 0] = lookup[nes_pixels[src_i] * 3 + 0];
                    screen_pixels[dst_i * 3 + 1] = lookup[nes_pixels[src_i] * 3 + 1];
                    screen_pixels[dst_i * 3 + 2] = lookup[nes_pixels[src_i] * 3 + 2];
                }
            }     

            // blit and render (vsync?)
            SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(screen_pixels, 256, 240, 24, 256 * 3, 0x0000FF, 0x00FF00, 0xFF0000, 0);
            assert(surface);
            SDL_BlitSurface(surface, 0, screen, 0);
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

    // free render
    SDL_FreeSurface(screen);
    SDL_DestroyWindow(window);
    free(screen_pixels);
    free(nes_pixels);

    // free nes
    free_nes(&nes);
}