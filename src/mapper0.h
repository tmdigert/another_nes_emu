#pragma once

#include <stdint.h>

// mapper 0
// Does no real mapping. IO routes to fixed banks. 
// PRG ROM sizes can be 0x4000 or 0x8000 bytes, cpu address 0x8000 routes to 0x0000. 
// CHR ROM sized is always 0x2000.
struct Mapper0 {
    uint8_t prg_rom[0x8000];
    uint16_t mask; // for mirroring
    uint8_t mirroring; // 0 = horizontal, 1 = vertical
    uint8_t chr_rom[0x2000];
};

uint8_t mapper0_prg_read(struct Mapper0*, uint16_t);
uint8_t mapper0_chr_read(struct Mapper0*, uint16_t);
void mapper0_prg_write(struct Mapper0*, uint16_t, uint8_t);