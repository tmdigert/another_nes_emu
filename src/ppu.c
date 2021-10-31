#include "nes.h"
#include <stdio.h>

uint8_t step_ppu(struct Nes* nes, uint8_t cycles) {
    // advance cycle
    uint32_t old_cycle = nes->cycle;
    nes->cycle += cycles;
    uint8_t vblank_flag = 0;

    // vblank occur scanline 241 cycle 1
    if (old_cycle <= 82182 && nes->cycle > 82182) {
        vblank_flag = 1;
        if (nes->ppuctrl & 0x80) {
            nes->nmi = 1;
        }
        printf("PPU: vblank (%i..%i) [nmi = %i, ppuctrl = 0x%02X]\n", old_cycle, nes->cycle, nes->nmi, nes->ppuctrl);
        nes->ppustatus |= 0x80; // set vblank flag
    }

    // vblank ends scanline 260, cycle 340
    if (nes->cycle >= 89342) {
        nes->cycle -= 89342;
    }

    return vblank_flag;
}