#pragma once

#include <stdint.h>

/* Cartridge
    Models an NES cartridge. Since cartridges can vary in size and layout, polymorphism is required, and function pointers
    are used to interface the cartridge data.
*/
struct Cartridge {
    void* data;
    uint8_t (*prg_read)(void*, uint16_t);
    uint8_t (*chr_read)(void*, uint16_t);
    void (*prg_write)(void*, uint16_t, uint8_t);
};

// Cartridge initialization/free functions.
int load_cartridge_from_file(char*, struct Cartridge*);
int load_cartridge_from_data(uint8_t*, uint8_t*, struct Cartridge*);
void free_cartridge(struct Cartridge*);

// Cartridge IO. prg_read and prg_write are used by the CPU, chr_read is used by the PPU.
uint8_t cartridge_prg_read(struct Cartridge*, uint16_t);
uint8_t cartridge_chr_read(struct Cartridge*, uint16_t);
void cartridge_prg_write(struct Cartridge*, uint16_t, uint8_t);