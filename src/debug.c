#include "debug.h"

void draw_ppu_nametables(struct Nes* nes, uint8_t pixels[4*256*240]) {
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
        for (uint8_t tile8_y = 0; tile8_y < NAMETABLE_HEIGHT; tile8_y++) {
            for (uint8_t tile8_x = 0; tile8_x < NAMETABLE_WIDTH; tile8_x++) {

                uint16_t nametable_index = tile8_x + tile8_y * NAMETABLE_WIDTH;
                uint16_t attribute_index = tile8_x / 4 + tile8_y / 4 * NAMETABLE_WIDTH / 4;

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
                for (uint8_t bit_plane = 0; bit_plane < TILE_SIZE; bit_plane++) {
                    // load the bit plane
                    uint8_t low_bits = ppu_bus_read(nes, pattern_table_base | bit_plane); // low bits for 8 pixels
                    uint8_t high_bits = ppu_bus_read(nes, pattern_table_base | TILE_SIZE | bit_plane); // high bits for 8 pixels

                    // for each pixel in plane
                    for (uint8_t pixel_index = 0; pixel_index < TILE_SIZE; pixel_index++) {
                        // extract rightmost pixel from the two bit planes above
                        uint8_t pixel = high_bits % 2 << 1 | low_bits % 2;
                        low_bits >>= 1;
                        high_bits >>= 1;

                        // find where this pixel is even suppose to go
                        uint16_t pixel_x = (TILE_SIZE - 1 - pixel_index) + TILE_SIZE * tile8_x + TILE_SIZE * NAMETABLE_WIDTH * (nametable % 2);
                        uint16_t pixel_y = TILE_SIZE * NAMETABLE_HEIGHT * (nametable / 2) + TILE_SIZE * tile8_y + bit_plane;
                        uint32_t index = pixel_x + pixel_y * 2 * NAMETABLE_WIDTH * TILE_SIZE;

                        // put
                        pixels[index] = bg_palette[palette_id][pixel];
                    }
                }
            }
        } 
    }
}