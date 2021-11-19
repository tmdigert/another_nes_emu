#include <stdio.h>

#include "nes.h"
#include "error.h"

#include <assert.h>

uint32_t cycle(uint16_t scanline, uint16_t cycle, uint8_t even) {
    return scanline * 341 + cycle + even;
}

#define NAMETABLE_WIDTH 32 // in tiles
#define NAMETABLE_HEIGHT 30 // in tiles
#define TILE_SIZE 8 // in pixels
#define SPRITE_SIZE 8 // in pixels

uint8_t step_ppu(struct Nes* nes, uint8_t cycles, uint8_t* pixels) {
    uint8_t vblank_flag = 0;

    // run each cycle individually
    for (uint8_t i = 0; i < cycles; i++, nes->cycle++) {
        if (nes->cycle == 89342) nes->cycle = 0;
        uint16_t scanline = nes->cycle / 341;
        uint16_t dot = nes->cycle % 341;

        // visible scanlines [0, 239]
        if (scanline <= 239) {
            if (dot > 255) continue; // skip
            uint8_t pixel_y = scanline;
            uint8_t pixel_x = dot;

            // get tile associated with pixel
            uint16_t tile8_x = pixel_x / TILE_SIZE;
            uint16_t tile8_y = pixel_y / TILE_SIZE;

            // address bases and indexes
            uint16_t nametable_base = 0x2000;
            uint16_t attribute_base = nametable_base | 0x3C0;
            uint16_t nametable_index = tile8_x + tile8_y * NAMETABLE_WIDTH;
            uint16_t attribute_index = tile8_x / 4 + tile8_y / 4 * NAMETABLE_WIDTH / 4;

            // data
            uint8_t nametable_data = ppu_bus_read(nes, nametable_base + nametable_index);
            uint8_t attribute_data = ppu_bus_read(nes, attribute_base + attribute_index);

            // pattern table
            uint16_t pattern_table_base = nametable_data << 4 | nes->ppuctrl << 8 & 0b1000000000000;

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
            if (pixel == 0) {
                palette_id = 0;
            }
            pixels[pixel_x + pixel_y * NAMETABLE_WIDTH * TILE_SIZE] = nes->palette[4 * palette_id + pixel]; 
            continue;
        }

        // [240] pre-render (idle)

        // [241, 260] vblank (mostly idle)
        if (nes->cycle == 82182) {
            vblank_flag = 1;
            nes->nmi = (nes->ppuctrl & 0x80) > 0;
            nes->ppustatus |= 0x80; // set vblank flag
            continue;
        }

        // [261] pre-render
    }

    return vblank_flag;

    // advance cycle
    /*uint32_t old_cycle = nes->cycle;
    nes->cycle += cycles;
    uint8_t vblank_flag = 0;

    // [0, 239] visible scanlines
    uint16_t NAMETABLE_WIDTH = 32; // in tiles
    uint16_t NAMETABLE_HEIGHT = 30; // in tiles
    uint16_t TILE_SIZE = 8; // in pixels
    uint16_t SPRITE_SIZE = 8; // in pixels
    for (uint32_t i = old_cycle; i < nes->cycle; i++) {
        if (i > cycle(240, 0, 0)) continue;
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

        // get pixel (x, y)
        uint32_t pixel_x = i % 341;
        uint32_t pixel_y = i / 341;
        
        // get tile associated with pixel
        uint16_t tile8_x = pixel_x / NAMETABLE_WIDTH;
        uint16_t tile8_y = pixel_y / NAMETABLE_HEIGHT;

        // address bases and indexes
        uint16_t nametable_base = 0x2000;
        uint16_t attribute_base = nametable_base | 0x3C0;
        uint16_t nametable_index = tile8_x + tile8_y * NAMETABLE_WIDTH;
        uint16_t attribute_index = tile8_x / 4 + tile8_y / 4 * NAMETABLE_HEIGHT / 4;

        // data
        uint8_t nametable_data = ppu_bus_read(nes, nametable_base + nametable_index);
        uint8_t attribute_data = ppu_bus_read(nes, attribute_base + attribute_index);

        // pattern table
        uint16_t pattern_table_base = nametable_data << 4 | nes->ppuctrl << 8 & 0b1000000000000;

        // palette
        uint8_t x_sub_tile = (tile8_x/2) % 2;
        uint8_t y_sub_tile = (tile8_y/2) % 2;
        uint8_t palette_id = (attribute_data >> x_sub_tile * 2 + y_sub_tile * 4) & 0b11;

        // get pixel data from above
        uint8_t bit_plane = pixel_y % 8;
        uint8_t low_bits = ppu_bus_read(nes, pattern_table_base | bit_plane); // low bits for 8 pixels
        uint8_t high_bits = ppu_bus_read(nes, pattern_table_base | TILE_SIZE | bit_plane); // high bits for 8 pixels
    
        // pixel
        uint8_t pixel_index = pixel_x % 8;
        uint8_t pixel = (high_bits >> pixel_index) % 2 << 1 | (low_bits >> pixel_index) % 2;

        // put
        pixels[pixel_x + pixel_y * NAMETABLE_WIDTH * TILE_SIZE] = bg_palette[palette_id][pixel];
    }

    // [240] post render scanline (idle)

    // [241, 260] vblank (idle (mostly))
    if (old_cycle <= 82182 && nes->cycle > 82182) {
        vblank_flag = 1;
        nes->nmi = nes->ppuctrl & 0x80 > 0;
        nes->ppustatus |= 0x80; // set vblank flag
    }

    // [261] pre-render
    if (nes->cycle >= 89342) {
        nes->cycle -= 89342;
    }

    return vblank_flag;*/
}