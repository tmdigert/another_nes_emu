#include <stdio.h>
#include <assert.h>

#include "nes.h"

uint16_t make_u16(uint8_t hi, uint8_t lo) {
    return ((uint16_t)hi << 8) | (uint16_t)lo;
}

void init_nes(struct Nes* nes, struct Cartridge cartridge) {
    // cartridge
    nes->cartridge = cartridge;

    // https://wiki.nesdev.org/w/index.php/CPU_power_up_state
    nes->acc = 0;
    nes->x = 0;
    nes->y = 0;
    nes->pc = 0;
    nes->sp = 0x00;
    nes->status = 0x34;
    nes->reset = 1; // the nes will perform a reset interrupt upon boot
    nes->nmi = 0;

    // https://wiki.nesdev.org/w/index.php/PPU_power_up_state
    nes->cycle = 0;
    nes->ppuctrl = 0;
    nes->ppumask = 0;
    nes->ppustatus = 0b00000000; // 1010_0000
    nes->oamaddr = 0;
    // TODO: ppulatch cleared
    nes->ppuscroll = 0;
    nes->ppuaddr = 0;
}

void free_nes(struct Nes* nes) {
    free_cartridge(&nes->cartridge);
}

void reset(struct Nes* nes) {
    nes->reset = 1;
}

uint8_t step_cpu(struct Nes* nes) {
    // perform reset if necessary
    if (nes->reset) {
        nes->reset = 0;
        
        // reset suppresses the writes done when pushing to the stack, so just dec stack pointer
        nes->sp -= 3;
        
        // read the reset vector (always starts at 0xFFFC)
        uint8_t lo = cpu_bus_read(nes, 0xFFFC);
        uint8_t hi = cpu_bus_read(nes, 0xFFFD);
        nes->pc = make_u16(hi, lo);

        // set flag
        set_flag(nes, STATUS_FLAG_INTERRUPT, 1);

        // all interrupts take 7 cycles
        return 7;
    }

    // perform nmi if necessary
    if (nes->nmi) {
        nes->nmi = 0;
        
        // push
        cpu_bus_write(nes, 0x0100 | nes->sp, nes->pc >> 8);
        nes->sp -= 1;
        cpu_bus_write(nes, 0x0100 | nes->sp, nes->pc & 0xFF);
        nes->sp -= 1;
        cpu_bus_write(nes, 0x0100 | nes->sp, nes->status);
        nes->sp -= 1;

        // read the nmi vector (always starts at 0xFFFA)
        uint8_t lo = cpu_bus_read(nes, 0xFFFA);
        uint8_t hi = cpu_bus_read(nes, 0xFFFB);
        nes->pc = make_u16(hi, lo);

        // set flag
        set_flag(nes, STATUS_FLAG_INTERRUPT, 1);

        // all interrupts take 7 cycles
        return 7;
    }

    // fetch first opcode
    uint8_t op = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;

    //
    uint8_t cycles = 0xFF; //
    uint8_t oops = 0x00;
    uint8_t branch = 0x00;
    switch (op) {
        // adc
        case 0x69: cycles = (addrm_imm(nes), adc(nes), 2); break;
        case 0x65: cycles = (addrm_zp(nes), adc(nes), 3); break;
        case 0x75: cycles = (addrm_zpx(nes), adc(nes), 4); break;
        case 0x6D: cycles = (addrm_abs(nes), adc(nes), 4); break;
        case 0x7D: cycles = (oops = addrm_abx(nes), adc(nes), 4 + oops); break;
        case 0x79: cycles = (oops = addrm_aby(nes), adc(nes), 4 + oops); break;
        case 0x61: cycles = (addrm_inx(nes), adc(nes), 6); break;
        case 0x71: cycles = (oops = addrm_iny(nes), adc(nes), 5 + oops); break;

        // and
        case 0x29: cycles = (addrm_imm(nes), and(nes), 2); break;
        case 0x25: cycles = (addrm_zp(nes), and(nes), 3); break;
        case 0x35: cycles = (addrm_zpx(nes), and(nes), 4); break;
        case 0x2D: cycles = (addrm_abs(nes), and(nes), 4); break;
        case 0x3D: cycles = (oops = addrm_abx(nes), and(nes), 4 + oops); break;
        case 0x39: cycles = (oops = addrm_aby(nes), and(nes), 4 + oops); break;
        case 0x21: cycles = (addrm_inx(nes), and(nes), 6); break;
        case 0x31: cycles = (oops = addrm_iny(nes), and(nes), 5 + oops); break;

        // asl
        case 0x0A: cycles = (asl_imp(nes), 2); break;
        case 0x06: cycles = (addrm_zp(nes), asl(nes), 5); break;
        case 0x16: cycles = (addrm_zpx(nes), asl(nes), 6); break;
        case 0x0E: cycles = (addrm_abs(nes), asl(nes), 6); break;
        case 0x1E: cycles = (addrm_abx(nes), asl(nes), 7); break;

        // bit
        case 0x24: cycles = (addrm_zp(nes), bit(nes), 3); break;
        case 0x2C: cycles = (addrm_abs(nes), bit(nes), 4); break;

        // branch
        case 0x10: cycles = (addrm_rel(nes), branch = bpl(nes), 2 + branch); break;
        case 0x30: cycles = (addrm_rel(nes), branch = bmi(nes), 2 + branch); break;
        case 0x50: cycles = (addrm_rel(nes), branch = bvc(nes), 2 + branch); break;
        case 0x70: cycles = (addrm_rel(nes), branch = bvs(nes), 2 + branch); break;
        case 0x90: cycles = (addrm_rel(nes), branch = bcc(nes), 2 + branch); break;
        case 0xB0: cycles = (addrm_rel(nes), branch = bcs(nes), 2 + branch); break;
        case 0xD0: cycles = (addrm_rel(nes), branch = bne(nes), 2 + branch); break;
        case 0xF0: cycles = (addrm_rel(nes), branch = beq(nes), 2 + branch); break;

        // brk
        case 0x00: cycles = (brk(nes), 7); break;

        // cmp
        case 0xC9: cycles = (addrm_imm(nes), cmp(nes), 2); break;
        case 0xC5: cycles = (addrm_zp(nes), cmp(nes), 3); break;
        case 0xD5: cycles = (addrm_zpx(nes), cmp(nes), 4); break;
        case 0xCD: cycles = (addrm_abs(nes), cmp(nes), 4); break;
        case 0xDD: cycles = (oops = addrm_abx(nes), cmp(nes), 4 + oops); break;
        case 0xD9: cycles = (oops = addrm_aby(nes), cmp(nes), 4 + oops); break;
        case 0xC1: cycles = (addrm_inx(nes), cmp(nes), 6); break;
        case 0xD1: cycles = (oops = addrm_iny(nes), cmp(nes), 5 + oops); break; 

        // cpx
        case 0xE0: cycles = (addrm_imm(nes), cpx(nes), 2); break;
        case 0xE4: cycles = (addrm_zp(nes), cpx(nes), 3); break;
        case 0xEC: cycles = (addrm_abs(nes), cpx(nes), 4); break;

        // cpy
        case 0xC0: cycles = (addrm_imm(nes), cpy(nes), 2); break;
        case 0xC4: cycles = (addrm_zp(nes), cpy(nes), 3); break;
        case 0xCC: cycles = (addrm_abs(nes), cpy(nes), 4); break;

        // dec
        case 0xC6: cycles = (addrm_zp(nes), dec(nes), 5); break;
        case 0xD6: cycles = (addrm_zpx(nes), dec(nes), 6); break;
        case 0xCE: cycles = (addrm_abs(nes), dec(nes), 6); break;
        case 0xDE: cycles = (addrm_abx(nes), dec(nes), 7); break;

        // eor
        case 0x49: cycles = (addrm_imm(nes), eor(nes), 2); break;
        case 0x45: cycles = (addrm_zp(nes), eor(nes), 3); break;
        case 0x55: cycles = (addrm_zpx(nes), eor(nes), 4); break;
        case 0x4D: cycles = (addrm_abs(nes), eor(nes), 4); break;
        case 0x5D: cycles = (oops = addrm_abx(nes), eor(nes), 4 + oops); break;
        case 0x59: cycles = (oops = addrm_aby(nes), eor(nes), 4 + oops); break;
        case 0x41: cycles = (addrm_inx(nes), eor(nes), 6); break;
        case 0x51: cycles = (oops = addrm_iny(nes), eor(nes), 5 + oops); break;

        // flag instructions
        case 0x18: cycles = (clc(nes), 2); break; //clc
        case 0x38: cycles = (sec(nes), 2); break; //sec
        case 0x58: cycles = (cli(nes), 2); break; //cli
        case 0x78: cycles = (sei(nes), 2); break; //sei
        case 0xB8: cycles = (clv(nes), 2); break; //clv
        case 0xD8: cycles = (cld(nes), 2); break; //cld
        case 0xF8: cycles = (sed(nes), 2); break; //sed

        // inc
        case 0xE6: cycles = (addrm_zp(nes), inc(nes), 5); break;
        case 0xF6: cycles = (addrm_zpx(nes), inc(nes), 6); break;
        case 0xEE: cycles = (addrm_abs(nes), inc(nes), 6); break;
        case 0xFE: cycles = (addrm_abx(nes), inc(nes), 7); break;

        // jmp
        case 0x4C: cycles = (addrm_abs(nes), jmp(nes), 3); break;
        case 0x6C: cycles = (addrm_ind(nes), jmp(nes), 5); break;

        // jsr
        case 0x20: cycles = (addrm_abs(nes), jsr(nes), 6); break;

        // lda
        case 0xA9: cycles = (addrm_imm(nes), lda(nes), 2); break;
        case 0xA5: cycles = (addrm_zp(nes), lda(nes), 3); break;
        case 0xB5: cycles = (addrm_zpx(nes), lda(nes), 4); break;
        case 0xAD: cycles = (addrm_abs(nes), lda(nes), 4); break;
        case 0xBD: cycles = (oops = addrm_abx(nes), lda(nes), 4 + oops); break;
        case 0xB9: cycles = (oops = addrm_aby(nes), lda(nes), 4 + oops); break;
        case 0xA1: cycles = (addrm_inx(nes), lda(nes), 6); break;
        case 0xB1: cycles = (oops = addrm_iny(nes), lda(nes), 5 + oops); break;

        // ldx
        case 0xA2: cycles = (addrm_imm(nes), ldx(nes), 2); break;
        case 0xA6: cycles = (addrm_zp(nes), ldx(nes), 3); break;
        case 0xB6: cycles = (addrm_zpy(nes), ldx(nes), 4); break;
        case 0xAE: cycles = (addrm_abs(nes), ldx(nes), 4); break;
        case 0xBE: cycles = (oops = addrm_aby(nes), ldx(nes), 4 + oops); break;

        // ldy
        case 0xA0: cycles = (addrm_imm(nes), ldy(nes), 2); break;
        case 0xA4: cycles = (addrm_zp(nes), ldy(nes), 3); break;
        case 0xB4: cycles = (addrm_zpx(nes), ldy(nes), 4); break;
        case 0xAC: cycles = (addrm_abs(nes), ldy(nes), 4); break;
        case 0xBC: cycles = (oops = addrm_abx(nes), ldy(nes), 4 + oops); break;

        // lsr
        case 0x4A: cycles = (lsr_imp(nes), 2); break;
        case 0x46: cycles = (addrm_zp(nes), lsr(nes), 5); break;
        case 0x56: cycles = (addrm_zpx(nes), lsr(nes), 6); break;
        case 0x4E: cycles = (addrm_abs(nes), lsr(nes), 6); break;
        case 0x5E: cycles = (addrm_abx(nes), lsr(nes), 7); break;

        // nop
        case 0xEA: cycles = (nop(nes), 2); break;
        case 0x1A: cycles = (nop(nes), 2); break;
        case 0x3A: cycles = (nop(nes), 2); break;
        case 0x5A: cycles = (nop(nes), 2); break;
        case 0x7A: cycles = (nop(nes), 2); break;
        case 0xDA: cycles = (nop(nes), 2); break;
        case 0xFA: cycles = (nop(nes), 2); break;
        case 0x04: cycles = (addrm_zp(nes), nop(nes), 3); break;
        case 0x44: cycles = (addrm_zp(nes), nop(nes), 3); break;
        case 0x64: cycles = (addrm_zp(nes), nop(nes), 3); break;
        case 0x0C: cycles = (addrm_abs(nes), nop(nes), 4); break;
        case 0x14: cycles = (addrm_zpx(nes), nop(nes), 4); break;
        case 0x34: cycles = (addrm_zpx(nes), nop(nes), 4); break;
        case 0x54: cycles = (addrm_zpx(nes), nop(nes), 4); break;
        case 0x74: cycles = (addrm_zpx(nes), nop(nes), 4); break;
        case 0xD4: cycles = (addrm_zpx(nes), nop(nes), 4); break;
        case 0xF4: cycles = (addrm_zpx(nes), nop(nes), 4); break;
        case 0x80: cycles = (addrm_imm(nes), nop(nes), 2); break;
        case 0x1C: cycles = (oops = addrm_abx(nes), nop(nes), 4 + oops); break; 
        case 0x3C: cycles = (oops = addrm_abx(nes), nop(nes), 4 + oops); break; 
        case 0x5C: cycles = (oops = addrm_abx(nes), nop(nes), 4 + oops); break; 
        case 0x7C: cycles = (oops = addrm_abx(nes), nop(nes), 4 + oops); break; 
        case 0xDC: cycles = (oops = addrm_abx(nes), nop(nes), 4 + oops); break; 
        case 0xFC: cycles = (oops = addrm_abx(nes), nop(nes), 4 + oops); break;


        // ora
        case 0x09: cycles = (addrm_imm(nes), ora(nes), 2); break;
        case 0x05: cycles = (addrm_zp(nes), ora(nes), 3); break;
        case 0x15: cycles = (addrm_zpx(nes), ora(nes), 4); break;
        case 0x0D: cycles = (addrm_abs(nes), ora(nes), 4); break;
        case 0x1D: cycles = (oops = addrm_abx(nes), ora(nes), 4 + oops); break;
        case 0x19: cycles = (oops = addrm_aby(nes), ora(nes), 4 + oops); break;
        case 0x01: cycles = (addrm_inx(nes), ora(nes), 6); break;
        case 0x11: cycles = (oops = addrm_iny(nes), ora(nes), 5 + oops); break;

        // register instructions
        case 0xAA: cycles = (tax(nes), 2); break; //tax
        case 0x8A: cycles = (txa(nes), 2); break; //txa
        case 0xCA: cycles = (dex(nes), 2); break; //dex
        case 0xE8: cycles = (inx(nes), 2); break; //inx
        case 0xA8: cycles = (tay(nes), 2); break; //tay
        case 0x98: cycles = (tya(nes), 2); break; //tya
        case 0x88: cycles = (dey(nes), 2); break; //dey
        case 0xC8: cycles = (iny(nes), 2); break; //iny

        // rol
        case 0x2A: cycles = (rol_imp(nes), 2); break;
        case 0x26: cycles = (addrm_zp(nes), rol(nes), 5); break;
        case 0x36: cycles = (addrm_zpx(nes), rol(nes), 6); break;
        case 0x2E: cycles = (addrm_abs(nes), rol(nes), 6); break;
        case 0x3E: cycles = (addrm_abx(nes), rol(nes),7); break;

        // ror
        case 0x6A: cycles = (ror_imp(nes), 2); break;
        case 0x66: cycles = (addrm_zp(nes), ror(nes), 5); break;
        case 0x76: cycles = (addrm_zpx(nes), ror(nes), 6); break;
        case 0x6E: cycles = (addrm_abs(nes), ror(nes), 6); break;
        case 0x7E: cycles = (addrm_abx(nes), ror(nes),7); break;

        // rti
        case 0x40: cycles = (rti(nes), 6); break;

        // rts
        case 0x60: cycles = (rts(nes), 6); break;

        // sbc
        case 0xE9: cycles = (addrm_imm(nes), sbc(nes), 2); break;
        case 0xE5: cycles = (addrm_zp(nes), sbc(nes), 3); break;
        case 0xF5: cycles = (addrm_zpx(nes), sbc(nes), 4); break;
        case 0xED: cycles = (addrm_abs(nes), sbc(nes), 4); break;
        case 0xFD: cycles = (oops = addrm_abx(nes), sbc(nes), 4 + oops); break;
        case 0xF9: cycles = (oops = addrm_aby(nes), sbc(nes), 4 + oops); break;
        case 0xE1: cycles = (addrm_inx(nes), sbc(nes), 6); break;
        case 0xF1: cycles = (oops = addrm_iny(nes), sbc(nes), 5 + oops); break;
        
        // sta
        case 0x85: cycles = (addrm_zp(nes), sta(nes), 3); break;
        case 0x95: cycles = (addrm_zpx(nes), sta(nes), 4); break;
        case 0x8D: cycles = (addrm_abs(nes), sta(nes), 4); break;
        case 0x9D: cycles = (addrm_abx(nes), sta(nes), 5); break;
        case 0x99: cycles = (addrm_aby(nes), sta(nes), 5); break;
        case 0x81: cycles = (addrm_inx(nes), sta(nes), 6); break;
        case 0x91: cycles = (addrm_iny(nes), sta(nes), 6); break; 

        //stack instructions
        case 0x9A: cycles = (txs(nes), 2); break; //txs
        case 0xBA: cycles = (tsx(nes), 2); break; //tsx
        case 0x48: cycles = (pha(nes), 3); break; //pha
        case 0x68: cycles = (pla(nes), 4); break; //pla
        case 0x08: cycles = (php(nes), 3); break; //php
        case 0x28: cycles = (plp(nes), 4); break; //plp

        // stx
        case 0x86: cycles = (addrm_zp(nes), stx(nes), 3); break;
        case 0x96: cycles = (addrm_zpy(nes), stx(nes), 4); break;
        case 0x8E: cycles = (addrm_abs(nes), stx(nes), 4); break;   

        // sty
        case 0x84: cycles = (addrm_zp(nes), sty(nes), 3); break;
        case 0x94: cycles = (addrm_zpx(nes), sty(nes), 4); break;
        case 0x8C: cycles = (addrm_abs(nes), sty(nes), 4); break;

        default:
        // unhandled opcode error
        printf("INVALID OPCODE %02X\n", op);
        fflush(stdout);
        assert(0);
        break;
    }

    return cycles;
}

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

void set_flag(struct Nes* nes, uint8_t n, uint8_t val) {
    nes->status &= ~(1 << n); // clear nth bit
    nes->status |= val << n; // set nth bit to val
}

uint8_t get_flag(struct Nes* nes, uint8_t n) {
    return (nes->status >> n) & 0x01;
}