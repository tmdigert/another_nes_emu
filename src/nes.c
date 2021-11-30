#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "nes.h"

uint16_t make_u16(uint8_t hi, uint8_t lo) {
    return ((uint16_t)hi << 8) | (uint16_t)lo;
}

void set_flag(struct Nes* nes, uint8_t n, uint8_t val) {
    nes->status &= ~(1 << n); // clear nth bit
    nes->status |= val << n; // set nth bit to val
}

uint8_t get_flag(struct Nes* nes, uint8_t n) {
    return (nes->status >> n) & 0x01;
}

void init_nes(struct Nes* nes, struct Cartridge cartridge) {
    // cartridge
    nes->cartridge = cartridge;

    //
    nes->input1 = 0;
    nes->input2 = 0;
    nes->joy1 = 0;
    nes->joy2 = 0;

    // https://wiki.nesdev.org/w/index.php/CPU_power_up_state
    memset(nes->ram, 0, 0x800); 
    nes->acc = 0;
    nes->x = 0;
    nes->y = 0;
    nes->pc = 0;
    nes->sp = 0x00;
    nes->status = 0x34;
    nes->reset = 1; // the nes will perform a reset interrupt upon boot
    nes->nmi = 0;
    nes->oam_delay = 0;

    // https://wiki.nesdev.org/w/index.php/PPU_power_up_state
    memset(nes->ciram, 0, 0x800);
    memset(nes->oam, 0, 0x100);
    memset(nes->palette, 0, 0x20);
    nes->cycle = 0;
    nes->ppuctrl = 0;
    nes->ppumask = 0;
    nes->ppustatus = 0b00000000; // 1010_0000
    nes->oamaddr = 0;
    // TODO: ppulatch cleared
    nes->ppuscroll = 0;
    nes->ppuaddr = 0;
    nes->read_buffer = 0;
}

void free_nes(struct Nes* nes) {
    free_cartridge(&nes->cartridge);
}

void reset(struct Nes* nes) {
    nes->reset = 1;
}

