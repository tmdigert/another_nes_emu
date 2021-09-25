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
    nes->status &= !(1 << n); // clear nth bit
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
	set_flag(nes, STATUS_FLAG_OVERFLOW, old_acc >> 7 ^ nes->>acc >> 7);
	set_flag(nes, STATUS_FLAG_CARRY, nes->acc < old_acc);
	set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
}

void sbc(struct Nes* nes) {
	uint8_t old_acc = nes->acc;
	uint8_t val = !cpu_read(nes, nes->micro_addr);
	nes->acc += val + (nes->status & 1 << STATUS_FLAG_CARRY > 0);

	set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
	set_flag(nes, STATUS_FLAG_OVERFLOW, old_acc >> 7 ^ nes->acc >> 7);
	set_flag(nes, STATUS_FLAG_CARRY, nes->acc < old_acc);
	set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
}