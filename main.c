#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>

#include "nes.h"

/*int load_rom(char* filename, uint8_t header[16], uint8_t** rom) {
    FILE* fp = fopen(filename, "r");
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    printf("Rom loaded with size %i\n", size);
    *rom = malloc(size - 16);
    fseek (fp , 0, SEEK_SET);
    fread(header, 1, 16, fp);
    fseek (fp , 16 , SEEK_SET);
    fread(*rom, 1, size - 16, fp);
    fclose(fp);
}

int main(int argc, char* argv[]) {
    // load ROM
    uint8_t header[16];
    uint8_t* rom = 0;
    load_rom("nestest.nes", header, &rom);

    // generate nes
    struct Nes nes;
    init_nes(&nes, rom);
    nes.pc = 0xC000;

    // step the nes 7 times (complete first brk)
    int cyc = 7;
    for (int instr = 1; instr <= 6000; instr++) {
        printf("%6i PC:%04X A:%02X X:%02X Y:%02X P:%02X, SP:%02X CYC:%i\n", instr, nes.pc, nes.acc, nes.x, nes.y, nes.status, nes.sp, cyc);
        fflush(stdout);
        cyc += step(&nes);
    }

    free_nes(&nes);
}*/

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

        // cpx
        case 0xE0: return "cpx";
        case 0xE4: return "cpx";
        case 0xEC: return "cpx";

        // cpy
        case 0xC0: return "cpy";
        case 0xC4: return "cpy";
        case 0xCC: return "cpy";

        // dec
        case 0xC6: return "dec";
        case 0xD6: return "dec";
        case 0xCE: return "dec";
        case 0xDE: return "dec";

        // eor
        case 0x49: return "eor";
        case 0x45: return "eor";
        case 0x55: return "eor";
        case 0x4D: return "eor";
        case 0x5D: return "eor";
        case 0x59: return "eor";
        case 0x41: return "eor";
        case 0x51: return "eor";

        // flag instructions
        case 0x18: return "clc";
        case 0x38: return "sec";
        case 0x58: return "cli";
        case 0x78: return "sei";
        case 0xB8: return "clv";
        case 0xD8: return "cld";
        case 0xF8: return "sed";

        // inc
        case 0xE6: return "inc";
        case 0xF6: return "inc";
        case 0xEE: return "inc";
        case 0xFE: return "inc";

        // jmp
        case 0x4C: return "jmp";
        case 0x6C: return "jmp";

        // jsr
        case 0x20: return "jsr";

        // lda
        case 0xA9: return "lda";
        case 0xA5: return "lda";
        case 0xB5: return "lda";
        case 0xAD: return "lda";
        case 0xBD: return "lda";
        case 0xB9: return "lda";
        case 0xA1: return "lda";
        case 0xB1: return "lda";

        // ldx
        case 0xA2: return "ldx";
        case 0xA6: return "ldx";
        case 0xB6: return "ldx";
        case 0xAE: return "ldx";
        case 0xBE: return "ldx";

        // ldy
        case 0xA0: return "ldy";
        case 0xA4: return "ldy";
        case 0xB4: return "ldy";
        case 0xAC: return "ldy";
        case 0xBC: return "ldy";

        // lsr
        case 0x4A: return "lsr";
        case 0x46: return "lsr";
        case 0x56: return "lsr";
        case 0x4E: return "lsr";
        case 0x5E: return "lsr";

        // nop
        case 0xEA: return "nop";
        case 0x1A: return "nop";
        case 0x3A: return "nop";
        case 0x5A: return "nop";
        case 0x7A: return "nop";
        case 0xDA: return "nop";
        case 0xFA: return "nop";
        case 0x04: return "nop";
        case 0x44: return "nop";
        case 0x64: return "nop";
        case 0x0C: return "nop";
        case 0x14: return "nop";
        case 0x34: return "nop";
        case 0x54: return "nop";
        case 0x74: return "nop";
        case 0xD4: return "nop";
        case 0xF4: return "nop";
        case 0x80: return "nop";
        case 0x1C: return "nop";
        case 0x3C: return "nop";
        case 0x5C: return "nop";
        case 0x7C: return "nop";
        case 0xDC: return "nop";
        case 0xFC: return "nop";


        // ora
        case 0x09: return "ora";
        case 0x05: return "ora";
        case 0x15: return "ora";
        case 0x0D: return "ora";
        case 0x1D: return "ora";
        case 0x19: return "ora";
        case 0x01: return "ora";
        case 0x11: return "ora";

        // register instructions
        case 0xAA: return "tax";
        case 0x8A: return "txa";
        case 0xCA: return "dex";
        case 0xE8: return "inx";
        case 0xA8: return "tay";
        case 0x98: return "tya";
        case 0x88: return "dey";
        case 0xC8: return "iny";

        // rol
        case 0x2A: return "rol";
        case 0x26: return "rol";
        case 0x36: return "rol";
        case 0x2E: return "rol";
        case 0x3E: return "rol";

        // ror
        case 0x6A: return "ror";
        case 0x66: return "ror";
        case 0x76: return "ror";
        case 0x6E: return "ror";
        case 0x7E: return "ror";

        // rti
        case 0x40: return "rti";

        // rts
        case 0x60: return "rts";

        // sbc
        case 0xE9: return "sbc";
        case 0xE5: return "sbc";
        case 0xF5: return "sbc";
        case 0xED: return "sbc";
        case 0xFD: return "sbc";
        case 0xF9: return "sbc";
        case 0xE1: return "sbc";
        case 0xF1: return "sbc";
        
        // sta
        case 0x85: return "sta";
        case 0x95: return "sta";
        case 0x8D: return "sta";
        case 0x9D: return "sta";
        case 0x99: return "sta";
        case 0x81: return "sta";
        case 0x91: return "sta";

        //stack instructions
        case 0x9A: return "txs";
        case 0xBA: return "tsx";
        case 0x48: return "pha";
        case 0x68: return "pla";
        case 0x08: return "php";
        case 0x28: return "plp";

        // stx
        case 0x86: return "stx";
        case 0x96: return "stx";
        case 0x8E: return "stx"; 

        // sty
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
    for (int i = 0; i < 8; i++) {
        int cycle = step(&nes);
        uint8_t next = cpu_read(&nes, nes.pc);
        printf("cycle: %8i, pc: 0x%04X, A/X/Y: $%02X/$%02X/$%02X, sp: $%02X, status: $%02X, next op: 0x%02X (%s)\n", cycle, nes.pc, nes.acc, nes.x, nes.y, nes.sp, nes.status, next, lookup_opcode(next));
        acc_cycle += cycle;
    }

    // free nes
    free_nes(&nes);
}