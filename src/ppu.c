#include <stdio.h>

#include "nes.h"
#include "error.h"
#include "defines.h"

#include <assert.h>

uint32_t cycle(uint16_t scanline, uint16_t cycle, uint8_t even) {
    return scanline * 341 + cycle + even;
}

uint8_t step_ppu(struct Nes* nes, uint8_t cycles, uint8_t* pixels) {
    uint8_t vblank_flag = 0;

    // Run each cycle individually. A bit slow, but the PPU can be hectic.
    for (uint8_t i = 0; i < cycles; i++, nes->cycle++) {
        if (nes->cycle == 89342) nes->cycle = 0;
        uint16_t scanline = nes->cycle / 341;
        uint16_t dot = nes->cycle % 341;

        // Range [0, 239]: visible scanlines.
        if (scanline <= 239) {
            if (dot >= 257 && dot <= 320) {
                nes->oamaddr = 0;
            }
            uint8_t sprite0_x = nes->oam[nes->oamaddr];
            uint8_t sprite0_y = nes->oam[(uint8_t)(nes->oamaddr + 3)] - 1;
            if (dot > 255) continue; // skip
            if (scanline == sprite0_y && dot == sprite0_x && (nes->ppustatus & 0b01000000) == 0) {
                // sprite 0 hit
                nes->ppustatus |= 0b01000000;
            }
            uint8_t pixel_y = scanline;
            uint8_t pixel_x = dot;

            // get tile associated with pixel
            uint16_t tile8_x = pixel_x / TILE_SIZE;
            uint16_t tile8_y = pixel_y / TILE_SIZE;

            // address bases and indexes
            uint16_t nametable_base = 0x2000 | (0x400 * (nes->ppuctrl & 0b11));
            uint16_t attribute_base = nametable_base | 0x3C0;
            uint16_t nametable_index = tile8_x + tile8_y * NAMETABLE_WIDTH;
            uint16_t attribute_index = tile8_x / 4 + tile8_y / 4 * NAMETABLE_WIDTH / 4;

            // data
            uint8_t nametable_data = ppu_bus_read(nes, nametable_base | nametable_index);
            uint8_t attribute_data = ppu_bus_read(nes, attribute_base | attribute_index);

            // pattern table
            uint16_t pattern_table_base = nametable_data << 4 | (((nes->ppuctrl >> 4) & 0x1) * 0x1000);

            // palette
            uint8_t x_sub_tile = (tile8_x/2) % 2;
            uint8_t y_sub_tile = (tile8_y/2) % 2;
            uint8_t palette_id = (attribute_data >> x_sub_tile * 2 + y_sub_tile * 4) & 0b11;

            // get pixel data from above
            uint8_t bit_plane = pixel_y % 8;
            uint8_t low_bits = ppu_bus_read(nes, pattern_table_base | bit_plane); // low bits for 8 pixels
            uint8_t high_bits = ppu_bus_read(nes, pattern_table_base | TILE_SIZE | bit_plane); // high bits for 8 pixels
    
            // pixel
            uint8_t pixel_index = TILE_SIZE - 1 - (pixel_x % 8);
            uint8_t pixel = (high_bits >> pixel_index) % 2 << 1 | (low_bits >> pixel_index) % 2;

            // put
            if (pixel == 0) palette_id = 0;
            uint8_t color = ppu_bus_read(nes, 0x3F00 + 4 * palette_id + pixel);
            pixels[pixel_x + pixel_y * NAMETABLE_WIDTH * TILE_SIZE] = color; 
            continue;
        }

        // Range [240]: post-render (idle).

        // Range [241, 260]: vblank (mostly idle).
        if (scanline == 241 && dot == 1) {
            vblank_flag = 1;
            nes->nmi = (nes->ppuctrl & 0x80) > 0;
            nes->ppustatus |= 0x80; // set vblank flag
            continue;
        }

        // Range: [261] pre-render.
        if (scanline == 261) {
            if (dot == 1) {
                nes->ppustatus = 0;
                continue;
            }
            if (dot >= 257 && dot <= 320) {
                nes->oamaddr = 0;
                continue;
            }
        }
    }

    return vblank_flag;
}