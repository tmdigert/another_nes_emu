#include "nes.h"

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
    // read zp address (absolute address, low and high byte)
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

void addrm_rel(struct Nes* nes) {
    // read oeprand (offset)
    uint8_t offset = (uint16_t)cpu_read(nes, nes->pc);
    nes->pc += 1;
    // store internal address
    nes->micro_addr = nes->pc + (int8_t)offset;
}

void addrm_ind(struct Nes* nes) {
    // read operand (absolute address low byte)
    uint8_t lo = cpu_read(nes, nes->pc);
    nes->pc += 1;
    // read operand (absolute address high byte)
    uint8_t hi = cpu_read(nes, nes->pc);
    nes->pc += 1;
    // 
    lo = cpu_read(nes, make_u16(hi, lo));
    hi = cpu_read(nes, make_u16(hi, lo + 1));
    // store internal address
    nes->micro_addr = make_u16(lo, hi);
}