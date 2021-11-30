#include <assert.h>
#include "cartridge.h"
#include "mapper0.h"

uint8_t mapper0_prg_read(struct Mapper0* mapper0, uint16_t addr) {
    assert(addr >= 0x4020);
    if (addr < 0x8000) return 0; // TODO: Is this correct? What goes here?
    return mapper0->prg_rom[(addr - 0x8000) & mapper0->mask];
}

uint8_t mapper0_chr_read(struct Mapper0* mapper0, uint16_t addr) {
    assert(addr < 0x2000);
    return mapper0->chr_rom[addr];
}

void mapper0_prg_write(struct Mapper0* mapper0, uint16_t addr, uint8_t data) {
    // Intentionally blank, mapper0 has no write support.
}