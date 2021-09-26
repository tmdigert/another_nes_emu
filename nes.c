#include "nes.h"

uint16_t make_u16(uint8_t hi, uint8_t lo) {
    return ((uint16_t)hi << 8) | (uint16_t)lo;
}

void init_nes(struct Nes* nes) {
    // https://wiki.nesdev.org/w/index.php/CPU_power_up_state
    nes->acc = 0;
    nes->x = 0;
    nes->y = 0;
    nes->sp = 0xFD;
    nes->status = 0x34;     
}

void free_nes(struct Nes* nes) { }

uint8_t step(struct Nes* nes) {
    // fetch first opcode
    uint8_t op = fetch_op(nes);

    //
    uint8_t cycles = 0xFF; //
    uint8_t oops = 0x00;
    switch (op) {
		case 0xE9: cycles = (addrm_imm(nes), sbc(nes), 2); break;
		case 0xE5: cycles = (addrm_zp(nes), sbc(nes), 3); break;
		case 0xF5: cycles = (addrm_zpx(nes), sbc(nes), 4); break;
		case 0xED: cycles = (addrm_abs(nes), sbc(nes), 4); break;
		case 0xFD: cycles = (oops = addrm_abx(nes), sbc(nes), 4 + oops); break;
		case 0xF9: cycles = (oops = addrm_aby(nes), sbc(nes), 4 + oops); break;
		case 0xE1: cycles = (addrm_inx(nes), sbc(nes), 6); break;
		case 0xF1: cycles = (oops = addrm_iny(nes), sbc(nes), 5 + oops); break;

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
        case 0x0A: cycles = (addrm_imp(nes), asl(nes), 2); break;
        case 0x06: cycles = (addrm_zp(nes), asl(nes), 5); break;
        case 0x16: cycles = (addrm_zpx(nes), asl(nes), 6); break;
        case 0x0E: cycles = (addrm_abs(nes), asl(nes), 6); break;
        case 0x1E: cycles = (addrm_abx(nes), asl(nes), 7); break;

        // bit
        case 0x24: cycles = (addrm_zp(nes), bit(nes), 3); break;
        case 0x2C: cycles = (addrm_abs(nes), bit(nes), 4); break;

        // branch TODO: finish branch instructions

        // brk
        case 0x00: cycles = (addrm_imp(nes), brk(nes), 7); break;

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
        case 0xDE: cycles = (addrm_abx(nes), dec(nes),7); break;

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
        case 0x18: cycles = (addrm_imp(nes), clc(nes), 2); break; //clc
        case 0x38: cycles = (addrm_imp(nes), sec(nes), 2); break; //sec
        case 0x58: cycles = (addrm_imp(nes), cli(nes), 2); break; //cli
        case 0x78: cycles = (addrm_imp(nes), sei(nes), 2); break; //sei
        case 0xB8: cycles = (addrm_imp(nes), clv(nes), 2); break; //clv
        case 0xD8: cycles = (addrm_imp(nes), cld(nes), 2); break; //cld
        case 0xF8: cycles = (addrm_imp(nes), sed(nes), 2); break; //sed

        // inc
        case 0xE6: cycles = (addrm_zp(nes), inc(nes), 5); break;
        case 0xF6: cycles = (addrm_zpx(nes), inc(nes), 6); break;
        case 0xEE: cycles = (addrm_abs(nes), inc(nes), 6); break;
        case 0xFE: cycles = (addrm_abx(nes), inc(nes),7); break;

        // jmp
        case 0x4C: cycles = (addrm_abs(nes), jmp(nes), 3); break;
        case 0x6C: cycles = (addrm_ind(nes), jmp(nes), 5); break;

        // jsr
        case 0x20: cycles = (addrm_abs(nes), jsr(nes), 6); break;

        // ldx
        case 0xA2: cycles = (addrm_imm(nes), ldx(nes), 2); break;
        case 0xA6: cycles = (addrm_zp(nes), ldx(nes), 3); break;
        case 0xB6: cycles = (addrm_zpy(nes), ldx(nes), 4); break;
        case 0xAE: cycles = (addrm_abs(nes), ldx(nes), 4); break;
        case 0xBE: cycles = (oops = addrm_aby(nes), ldx(nes), 4 + oops); break;

        // ldy
        case 0xA0: cycles = (addrm_imm(nes), ldy(nes), 2); break;
        case 0xA4: cycles = (addrm_zp(nes), ldy(nes), 3); break;
        case 0xB4: cycles = (addrm_zpy(nes), ldy(nes), 4); break;
        case 0xAC: cycles = (addrm_abs(nes), ldy(nes), 4); break;
        case 0xBC: cycles = (oops = addrm_aby(nes), ldy(nes), 4 + oops); break;

        // lsr
        case 0xC6: cycles = (addrm_imp(nes), lsr(nes), 2); break;
        case 0x46: cycles = (addrm_zp(nes), lsr(nes), 5); break;
        case 0x56: cycles = (addrm_zpx(nes), lsr(nes), 6); break;
        case 0x4E: cycles = (addrm_abs(nes), lsr(nes), 6); break;
        case 0x5E: cycles = (addrm_abx(nes), lsr(nes),7); break;

        // nop
        case 0xEA: cycles = (addrm_imp(nes), nop(nes), 2); break;

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
        case 0xAA: cycles = (addrm_imp(nes), tax(nes), 2); break; //tax
        case 0x8A: cycles = (addrm_imp(nes), txa(nes), 2); break; //txa
        case 0xCA: cycles = (addrm_imp(nes), dex(nes), 2); break; //dex
        case 0xE8: cycles = (addrm_imp(nes), inx(nes), 2); break; //inx
        case 0xA8: cycles = (addrm_imp(nes), tay(nes), 2); break; //tay
        case 0x98: cycles = (addrm_imp(nes), tya(nes), 2); break; //tya
        case 0x88: cycles = (addrm_imp(nes), dey(nes), 2); break; //dey
        case 0xC8: cycles = (addrm_imp(nes), iny(nes), 2); break; //iny

        // rol
        case 0x2A: cycles = (addrm_imp(nes), rol(nes), 2); break;
        case 0x26: cycles = (addrm_zp(nes), rol(nes), 5); break;
        case 0x36: cycles = (addrm_zpx(nes), rol(nes), 6); break;
        case 0x2E: cycles = (addrm_abs(nes), rol(nes), 6); break;
        case 0x3E: cycles = (addrm_abx(nes), rol(nes),7); break;

        // ror
        case 0x6A: cycles = (addrm_imp(nes), ror(nes), 2); break;
        case 0x66: cycles = (addrm_zp(nes), ror(nes), 5); break;
        case 0x76: cycles = (addrm_zpx(nes), ror(nes), 6); break;
        case 0x6E: cycles = (addrm_abs(nes), ror(nes), 6); break;
        case 0x7E: cycles = (addrm_abx(nes), ror(nes),7); break;

        // rti
        case 0x40: cycles = (addrm_imp(nes), rti(nes), 6); break;

        // rts
        case 0x60: cycles = (addrm_imp(nes), rts(nes), 6); break;
        
        //stack instructions
        case 0x9A: cycles = (addrm_imp(nes), txs(nes), 2); break; //txs
        case 0xBA: cycles = (addrm_imp(nes), tsx(nes), 2); break; //tsx
        case 0x48: cycles = (addrm_imp(nes), pha(nes), 3); break; //pha
        case 0x68: cycles = (addrm_imp(nes), pla(nes), 4); break; //pla
        case 0x08: cycles = (addrm_imp(nes), php(nes), 3); break; //php
        case 0x28: cycles = (addrm_imp(nes), plp(nes), 4); break; //plp

        // sta
        case 0x85: cycles = (addrm_zp(nes), sta(nes), 3); break;
        case 0x95: cycles = (addrm_zpx(nes), sta(nes), 4); break;
        case 0x8D: cycles = (addrm_abs(nes), sta(nes), 4); break;
        case 0x9D: cycles = (addrm_abx(nes), sta(nes), 5); break;
        case 0x99: cycles = (addrm_aby(nes), sta(nes), 5); break;
        case 0x81: cycles = (addrm_inx(nes), sta(nes), 6); break;
        case 0x91: cycles = (addrm_iny(nes), sta(nes), 6); break;   

        // stx
        case 0x86: cycles = (addrm_zp(nes), stx(nes), 3); break;
        case 0x96: cycles = (addrm_zpy(nes), stx(nes), 4); break;
        case 0x8E: cycles = (addrm_abs(nes), stx(nes), 4); break;   

        // sty
        case 0x84: cycles = (addrm_zp(nes), sty(nes), 3); break;
        case 0x94: cycles = (addrm_zpx(nes), sty(nes), 4); break;
        case 0x8C: cycles = (addrm_abs(nes), sty(nes), 4); break;

        // lda
        case 0xA9: cycles = (addrm_imm(nes), lda(nes), 2); break;
        case 0xA5: cycles = (addrm_zp(nes), lda(nes), 3); break;
        case 0xB5: cycles = (addrm_zpx(nes), lda(nes), 4); break;
        case 0xAD: cycles = (addrm_abs(nes), lda(nes), 4); break;
        case 0xBD: cycles = (oops = addrm_abx(nes), lda(nes), 4 + oops); break;
        case 0xB9: cycles = (oops = addrm_abx(nes), lda(nes), 4 + oops); break;
        case 0xA1: cycles = (addrm_inx(nes), lda(nes), 6); break;
        case 0xB1: cycles = (oops = addrm_iny(nes), lda(nes), 5 + oops); 
        break;


        default:
        // unhandled opcode error
        break;
    }

    return cycles;
}

void set_flag(struct Nes* nes, uint8_t n, uint8_t val) {
    nes->status &= ~(1 << n); // clear nth bit
    nes->status |= val << n; // set nth bit to val
}

uint8_t cpu_read(struct Nes* nes, uint16_t addr) {
    return 0;
}

void cpu_write(struct Nes* nes, uint16_t addr, uint8_t byte) { 
}

uint8_t fetch_op(struct Nes* nes) {
    uint8_t op = cpu_read(nes, nes->pc);
    nes->pc += 1;
    return op;
}

void addrm_imp(struct Nes* nes) {
    // this one kind of sucks
}

void addrm_imm(struct Nes* nes) {
    // store internal address as operand address
    nes->micro_addr = nes->pc;
    nes->pc += 1;
}

void addrm_zp(struct Nes* nes) {
    // read operand (zp address), store
    nes->micro_addr = (uint16_t)cpu_read(nes, nes->pc);
    nes->pc += 1;
}

void addrm_zpx(struct Nes* nes) {
    // read operand (zp address)
    uint8_t addr = cpu_read(nes, nes->pc);
    nes->pc += 1;
    // inc zp address (by y)
    addr += nes->x;
    // store internal address
    nes->micro_addr = (uint16_t)addr;
}

void addrm_zpy(struct Nes* nes) {
    // read operand (zp address)
    uint8_t addr = (uint16_t)cpu_read(nes, nes->pc);
    nes->pc += 1;
    // inc zp address (by y)
    addr += nes->y;
    // store internal address
    nes->micro_addr = (uint16_t)addr;
}

void addrm_inx(struct Nes* nes) {
    // read operand (zp address)
    uint8_t addr = cpu_read(nes, nes->pc);
    nes->pc += 1;
    // inc zp address (by x)
    addr += nes->x;
    // read zp address (absolute address, low and high byte_)
    uint8_t lo = cpu_read(nes, addr);
    uint8_t hi = cpu_read(nes, addr + 1);
    // store internal address
    nes->micro_addr = make_u16(hi, lo);
}

uint8_t addrm_iny(struct Nes* nes) {
    // read operand (zp address) 
    uint8_t addr = cpu_read(nes, nes->pc);
    nes->pc += 1;
    // read zp address (absolute address, low and high byte)
    uint8_t lo = cpu_read(nes, addr);
    uint8_t hi = cpu_read(nes, addr + 1);
    // inc absolute address (by y), record carry
    uint8_t carry = lo + nes->x < lo;
    lo += nes->y;
    hi += carry;
    // store internal address
    nes->micro_addr = make_u16(hi, lo);
    return carry;
}

void addrm_abs(struct Nes* nes) {
    // read operand (absolute address low byte)
    uint8_t lo = cpu_read(nes, nes->pc);
    nes->pc += 1;
    // read operand (absolute address high byte)
    uint8_t hi = cpu_read(nes, nes->pc);
    nes->pc += 1;
    // store internal address
    nes->micro_addr = make_u16(hi, lo);
}

uint8_t addrm_abx(struct Nes* nes) {
    // read operand (absolute address low byte)
    uint8_t lo = cpu_read(nes, nes->pc);
    nes->pc += 1;
    // read operand (absolute address high byte)
    uint8_t hi = cpu_read(nes, nes->pc);
    nes->pc += 1;
    // inc absolute address (by x), record carry
    uint8_t carry = lo + nes->x < lo;
    lo += nes->x;
    hi += carry;
    // store internal address
    nes->micro_addr = make_u16(hi, lo);
    return carry;
}

uint8_t addrm_aby(struct Nes* nes) {
    // read operand (absolute address low byte)
    uint8_t lo = cpu_read(nes, nes->pc);
    nes->pc += 1;
    // read operand (absolute address high byte)
    uint8_t hi = cpu_read(nes, nes->pc);
    nes->pc += 1;
    // inc absolute address (by y), record carry
    uint8_t carry = lo + nes->y < lo;
    lo += nes->y;
    hi += carry;
    // store internal address
    nes->micro_addr = make_u16(hi, lo);
    return carry;
}

void lda(struct Nes* nes) {
    nes->acc = cpu_read(nes, nes->micro_addr);
    
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
}

void sta(struct Nes* nes) {
    cpu_write(nes, nes->micro_addr, nes->acc);
}

void stx(struct Nes* nes) {
    cpu_write(nes, nes->micro_addr, nes->x);
}

void sty(struct Nes* nes) {
    cpu_write(nes, nes->micro_addr, nes->y);
}

void adc(struct Nes* nes) {
	uint8_t old_acc = nes->acc;
	uint8_t val = cpu_read(nes, nes->micro_addr);
	nes->acc += val + (nes->status & (1 << STATUS_FLAG_CARRY) > 0);

	set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
	set_flag(nes, STATUS_FLAG_OVERFLOW, old_acc >> 7 ^ nes->acc >> 7);
	set_flag(nes, STATUS_FLAG_CARRY, nes->acc < old_acc);
	set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
}

void sbc(struct Nes* nes) {
	uint8_t old_acc = nes->acc;
	uint8_t val = ~cpu_read(nes, nes->micro_addr);
	nes->acc += val + (nes->status & 1 << STATUS_FLAG_CARRY > 0);

	set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
	set_flag(nes, STATUS_FLAG_OVERFLOW, old_acc >> 7 ^ nes->acc >> 7);
	set_flag(nes, STATUS_FLAG_CARRY, nes->acc < old_acc);
	set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
}
