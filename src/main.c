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

void fill_nametable(struct Nes* nes, uint8_t* pixels) {
    // constants
    uint16_t nametable_width = 32; // in tiles
    uint16_t nametable_height = 30; // in tiles
    uint16_t tile_size = 8; // in pixels

    // palettes
    uint8_t bg_palette[4][4];
    bg_palette[0][0] = ppu_bus_read(nes, 0x3F00);
    bg_palette[0][1] = ppu_bus_read(nes, 0x3F01);
    bg_palette[0][2] = ppu_bus_read(nes, 0x3F02);
    bg_palette[0][3] = ppu_bus_read(nes, 0x3F03);
    bg_palette[1][0] = ppu_bus_read(nes, 0x3F04);
    bg_palette[1][1] = ppu_bus_read(nes, 0x3F05);
    bg_palette[1][2] = ppu_bus_read(nes, 0x3F06);
    bg_palette[1][3] = ppu_bus_read(nes, 0x3F07);
    bg_palette[2][0] = ppu_bus_read(nes, 0x3F08);
    bg_palette[2][1] = ppu_bus_read(nes, 0x3F09);
    bg_palette[2][2] = ppu_bus_read(nes, 0x3F0A);
    bg_palette[2][3] = ppu_bus_read(nes, 0x3F0B);
    bg_palette[3][0] = ppu_bus_read(nes, 0x3F0C);
    bg_palette[3][1] = ppu_bus_read(nes, 0x3F0D);
    bg_palette[3][2] = ppu_bus_read(nes, 0x3F0E);
    bg_palette[3][3] = ppu_bus_read(nes, 0x3F0F);

    for (int nametable = 0; nametable < 4; nametable += 1) {
        uint16_t nametable_base = 0x2000 | (0x400 * nametable);
        uint16_t attribute_base = nametable_base | 0x3C0;

        // loop through each 8x8 tile (32 by 30 iterations)
        for (uint8_t tile8_y = 0; tile8_y < nametable_height; tile8_y++) {
            for (uint8_t tile8_x = 0; tile8_x < nametable_width; tile8_x++) {

                uint16_t nametable_index = tile8_x + tile8_y * nametable_width;
                uint16_t attribute_index = tile8_x / 4 + tile8_y / 4 * nametable_width / 4;

                // data from nametable/attribute tables
                uint8_t nametable_data = ppu_bus_read(nes, nametable_base + nametable_index);
                uint8_t attribute_data = ppu_bus_read(nes, attribute_base + attribute_index);

                // form patterntable base
                uint16_t pattern_table_base = nametable_data << 4 | nes->ppuctrl << 8 & 0b1000000000000; 

                // form palette id
                uint8_t x_sub_tile = (tile8_x/2) % 2;
                uint8_t y_sub_tile = (tile8_y/2) % 2;
                uint8_t palette_id = (attribute_data >> x_sub_tile * 2 + y_sub_tile * 4) & 0b11;

                // loop through each pixel (8 by 8 iterations)
                for (uint8_t bit_plane = 0; bit_plane < tile_size; bit_plane++) {
                    // load the bit plane
                    uint8_t low_bits = ppu_bus_read(nes, pattern_table_base | bit_plane); // low bits for 8 pixels
                    uint8_t high_bits = ppu_bus_read(nes, pattern_table_base | tile_size | bit_plane); // high bits for 8 pixels

                    // for each pixel in plane
                    for (uint8_t pixel_index = 0; pixel_index < tile_size; pixel_index++) {
                        // extract rightmost pixel from the two bit planes above
                        uint8_t pixel = high_bits % 2 << 1 | low_bits % 2;
                        low_bits >>= 1;
                        high_bits >>= 1;

                        // find where this pixel is even suppose to go
                        uint16_t pixel_x = (tile_size - 1 - pixel_index) + tile_size * tile8_x + tile_size * nametable_width * (nametable % 2);
                        uint16_t pixel_y = tile_size * nametable_height * (nametable / 2) + tile_size * tile8_y + bit_plane;
                        uint32_t index = pixel_x + pixel_y * 2 * nametable_width * tile_size;

                        // put
                        pixels[index] = bg_palette[palette_id][pixel];
                    }
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
    if (load_cartridge_from_file("donkeykong.nes", &cartridge) > 0) return -1;

    // load nes
    struct Nes nes;
    init_nes(&nes, cartridge);

    // run n vblanks of rom
    int acc_cycle = 7;
    int vblanks = 0;
    while (vblanks < 5) {
        uint8_t next = cpu_bus_read(&nes, nes.pc);
        //nlog("%6i  %04X  %02X    %s                             A:%02X X:%02X Y:%02X P:%02X SP:%02X             CYC:%i\n", i, nes.pc, next, lookup_opcode(next), nes.acc, nes.x, nes.y, nes.status, nes.sp, acc_cycle);

        int cycle = step_cpu(&nes);
        vblanks += step_ppu(&nes, cycle);
        acc_cycle += cycle;
    }

    /////////////////////
    // render nametable test
    uint32_t pixelc = (32 * 8 * 2) * (30 * 8 * 2);
    uint8_t* pixels = malloc(pixelc * 3);
    uint8_t* nes_pixels = malloc(pixelc);
    memset(nes_pixels, 0, pixelc);
    uint8_t lookup[] = {
        84, 84, 84, 0, 30, 116, 8, 16, 144, 48, 0, 136, 68, 0, 100, 92, 0, 48, 84, 4, 0, 60, 24, 0, 32, 42, 0, 8, 58, 0, 0, 64, 0, 0, 60, 0, 0, 50, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        152, 150, 152, 8, 76, 196, 48, 50, 236, 92, 30, 228, 136, 20, 176, 160, 20, 100, 152, 34, 32, 120, 60, 0, 84, 90, 0, 40, 114, 0, 8, 124, 0, 0, 118, 40, 0, 102, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        236, 238, 236, 76, 154, 236, 120, 124, 236, 176, 98, 236, 228, 84, 236, 236, 88, 180, 236, 106, 100, 212, 136, 32, 160, 170, 0, 116, 196, 0, 76, 208, 32, 56, 204, 108, 56, 180, 204, 60, 60, 60, 0, 0, 0, 0, 0, 0, 
        236, 238, 236, 168, 204, 236, 188, 188, 236, 212, 178, 236, 236, 174, 236, 236, 174, 212, 236, 180, 176, 228, 196, 144, 204, 210, 120, 180, 222, 120, 168, 226, 144, 152, 226, 180, 160, 214, 228, 160, 162, 160, 0, 0, 0, 0, 0, 0
    };
    fill_nametable(&nes, nes_pixels);
    // convert NES pixel to RGB pixels
    for (int i = 0; i < 245760; i++) {
        uint8_t nes_pixel = nes_pixels[i];
        pixels[i * 3 + 0] = lookup[nes_pixel * 3 + 0];
        pixels[i * 3 + 1] = lookup[nes_pixel * 3 + 1];
        pixels[i * 3 + 2] = lookup[nes_pixel * 3 + 2];
    }

    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
    SDL_Window* window = SDL_CreateWindow("NES Emu", 1920/2, 1080/2, 512, 480, 0);
    assert(window);
    SDL_Surface* screen = SDL_GetWindowSurface(window);
    assert(screen);
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(pixels, 512, 480, 24, 512 * 3, 0x0000FF, 0x00FF00, 0xFF0000, 0);
    assert(surface);
    SDL_BlitSurface(surface, 0, screen, 0);
    SDL_UpdateWindowSurface(window);
    system("PAUSE");
    SDL_FreeSurface(surface);
    SDL_FreeSurface(screen);
    SDL_DestroyWindow(window);
    free(pixels);
    free(nes_pixels);
    /////////////////////

    // free nes
    free_nes(&nes);
}