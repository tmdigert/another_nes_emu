#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>

#include "nes.h"

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

int main(int argc, char* argv[]) {
    // load ROM
    struct Cartridge cartridge;
    if (load_cartridge_from_file("donkeykong.nes", &cartridge) == -1) {
        printf("Failed to load cartridge: %s\n", "donkeykong.nes");
        return -1;
    }

    // load nes
    struct Nes nes;
    init_nes(&nes, cartridge);

    // do
    int acc_cycle = 0;
    for (int i = 0; i < 15; i++) {
        int cycle = step(&nes);
        uint8_t next = cpu_read(&nes, nes.pc);
        printf("cycle: %8i, pc: 0x%04X, A/X/Y: $%02X/$%02X/$%02X, sp: $%02X, status: $%02X, next op: 0x%02X (%s)\n", cycle, nes.pc, nes.acc, nes.x, nes.y, nes.sp, nes.status, next, lookup_opcode(next));
        acc_cycle += cycle;
    }

    // free nes
    free_nes(&nes);
}