#include "nes.h"
#include <stdio.h>

void addrm_imm(struct Nes* nes) {
    // store internal address as operand address
    nes->micro_addr = nes->pc;
    nes->pc += 1;
}

void addrm_zp(struct Nes* nes) {
    // read operand (zp address), store
    nes->micro_addr = (uint16_t)cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

void addrm_zpx(struct Nes* nes) {
    // read operand (zp address)
    uint8_t addr = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
    // inc zp address (by y)
    addr += nes->x;
    // store internal address
    nes->micro_addr = (uint16_t)addr;
}

void addrm_zpy(struct Nes* nes) {
    // read operand (zp address)
    uint8_t addr = (uint16_t)cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
    // inc zp address (by y)
    addr += nes->y;
    // store internal address
    nes->micro_addr = (uint16_t)addr;
}

void addrm_inx(struct Nes* nes) {
    // read operand (zp address)
    uint8_t addr = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
    // inc zp address (by x)
    addr += nes->x;
    // read zp address (absolute address, low and high byte)
    uint8_t lo = cpu_bus_read(nes, addr);
    uint8_t hi = cpu_bus_read(nes, (uint8_t)(addr + 1));
    // store internal address
    nes->micro_addr = make_u16(hi, lo);
}

uint8_t addrm_iny(struct Nes* nes) {
    // read operand (zp address) 
    uint8_t addr = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
    // read zp address (absolute address, low and high byte)
    uint8_t lo = cpu_bus_read(nes, addr);
    uint8_t hi = cpu_bus_read(nes, (uint8_t)(addr + 1));
    // inc absolute address (by y), record carry
    uint8_t carry = (uint8_t)(lo + nes->y) < lo;
    lo += nes->y;
    hi += carry;
    // store internal address
    nes->micro_addr = make_u16(hi, lo);
    return carry;
}

void addrm_abs(struct Nes* nes) {
    // read operand (absolute address low byte)
    uint8_t lo = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
    // read operand (absolute address high byte)
    uint8_t hi = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
    // store internal address
    nes->micro_addr = make_u16(hi, lo);
}

uint8_t addrm_abx(struct Nes* nes) {
    // read operand (absolute address low byte)
    uint8_t lo = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
    // read operand (absolute address high byte)
    uint8_t hi = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
    // inc absolute address (by x), record carry
    uint8_t carry = (uint8_t)(lo + nes->x) < lo;
    lo += nes->x;
    hi += carry;
    // store internal address
    nes->micro_addr = make_u16(hi, lo);
    return carry;
}

uint8_t addrm_aby(struct Nes* nes) {
    // read operand (absolute address low byte)
    uint8_t lo = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
    // read operand (absolute address high byte)
    uint8_t hi = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
    // inc absolute address (by y), record carry
    uint8_t carry = (uint8_t)(lo + nes->y) < lo;
    lo += nes->y;
    hi += carry;
    // store internal address
    nes->micro_addr = make_u16(hi, lo);
    return carry;
}

void addrm_rel(struct Nes* nes) {
    // read oeprand (offset)
    uint8_t offset = (uint16_t)cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
    // store internal address
    nes->micro_addr = nes->pc + (int8_t)offset;
}

void addrm_ind(struct Nes* nes) {
    // read operand (absolute address low byte)
    uint8_t lo = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
    // read operand (absolute address high byte)
    uint8_t hi = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
    // 
    uint8_t der_lo = cpu_bus_read(nes, make_u16(hi, lo));
    uint8_t der_hi = cpu_bus_read(nes, make_u16(hi, lo + 1));
    // store internal address
    nes->micro_addr = make_u16(der_hi, der_lo);
}

void adc(struct Nes* nes) {
    uint8_t val = cpu_bus_read(nes, nes->micro_addr);
    uint8_t old_acc = nes->acc;
    nes->acc += val + get_flag(nes, STATUS_FLAG_CARRY);

    uint8_t overflow = (val ^ old_acc) < 0x80 && (val ^ nes->acc) >> 7;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_OVERFLOW, overflow);
    set_flag(nes, STATUS_FLAG_CARRY, nes->acc <= old_acc);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
}

void and(struct Nes* nes) {
    nes->acc &= cpu_bus_read(nes, nes->micro_addr);

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
}

void asl(struct Nes* nes) {
    uint8_t val = cpu_bus_read(nes, nes->micro_addr);
    uint8_t bit7 = val >> 7;
    val <<= 1;
    cpu_bus_write(nes, nes->micro_addr, val);

    set_flag(nes, STATUS_FLAG_NEGATIVE, val >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, val == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit7);
}

void asl_imp(struct Nes* nes) {
    uint8_t bit7 = nes->acc >> 7;
    nes->acc <<= 1;

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit7);
}

void bit(struct Nes* nes) {
    uint8_t val = cpu_bus_read(nes, nes->micro_addr);
    uint8_t bit7 = val >> 7;
    uint8_t bit6 = (val >> 6) & 0x01;

    set_flag(nes, STATUS_FLAG_NEGATIVE, bit7);
    set_flag(nes, STATUS_FLAG_OVERFLOW, bit6);
    set_flag(nes, STATUS_FLAG_ZERO, (val & nes->acc) == 0);
}

uint8_t bpl(struct Nes* nes) {
    uint8_t neg_flag = get_flag(nes, STATUS_FLAG_NEGATIVE);
    if (!neg_flag) {
        uint8_t cross = (nes->pc ^ nes->micro_addr) > 0xFF;
        nes->pc = nes->micro_addr;
        return 1 + cross;
    }
    return 0;
}

uint8_t bmi(struct Nes* nes) {
    uint8_t neg_flag = get_flag(nes, STATUS_FLAG_NEGATIVE);
    if (neg_flag) {
        uint8_t cross = (nes->pc ^ nes->micro_addr) > 0xFF;
        nes->pc = nes->micro_addr;
        return 1 + cross;
    }
    return 0;
}

uint8_t bvc(struct Nes* nes) {
    uint8_t overflow_flag = get_flag(nes, STATUS_FLAG_OVERFLOW);
    if (!overflow_flag) {
        uint8_t cross = (nes->pc ^ nes->micro_addr) > 0xFF;
        nes->pc = nes->micro_addr;
        return 1 + cross;
    }
    return 0;
}

uint8_t bvs(struct Nes* nes) {
    uint8_t overflow_flag = get_flag(nes, STATUS_FLAG_OVERFLOW);
    if (overflow_flag) {
        uint8_t cross = (nes->pc ^ nes->micro_addr) > 0xFF;
        nes->pc = nes->micro_addr;
        return 1 + cross;
    }
    return 0;
}

uint8_t bcc(struct Nes* nes) {
    uint8_t carry_flag = get_flag(nes, STATUS_FLAG_CARRY);
    if (!carry_flag) {
        uint8_t cross = (nes->pc ^ nes->micro_addr) > 0xFF;
        nes->pc = nes->micro_addr;
        return 1 + cross;
    }
    return 0;
}

uint8_t bcs(struct Nes* nes) {
    uint8_t carry_flag = get_flag(nes, STATUS_FLAG_CARRY);
    if (carry_flag) {
        uint8_t cross = (nes->pc ^ nes->micro_addr) > 0xFF;
        nes->pc = nes->micro_addr;
        return 1 + cross;
    }
    return 0;
}

uint8_t bne(struct Nes* nes) {
    uint8_t zero_flag = get_flag(nes, STATUS_FLAG_ZERO);
    if (!zero_flag) {
        uint8_t cross = (nes->pc ^ nes->micro_addr) > 0xFF;
        nes->pc = nes->micro_addr;
        return 1 + cross;
    }
    return 0;
}

uint8_t beq(struct Nes* nes) {
    uint8_t zero_flag = get_flag(nes, STATUS_FLAG_ZERO);
    if (zero_flag) {
        uint8_t cross = (nes->pc ^ nes->micro_addr) > 0xFF;
        nes->pc = nes->micro_addr;
        return 1 + cross;
    }
    return 0;
}

void brk(struct Nes* nes) {
    // TODO: implement
}

// carry unset

void cmp(struct Nes* nes) {
    uint8_t val = cpu_bus_read(nes, nes->micro_addr);
    uint8_t dummy_acc = nes->acc + ~val + 1;

    set_flag(nes, STATUS_FLAG_NEGATIVE, dummy_acc >> 7);
    set_flag(nes, STATUS_FLAG_CARRY, dummy_acc <= nes->acc);
    set_flag(nes, STATUS_FLAG_ZERO, dummy_acc == 0);
}

void cpx(struct Nes* nes) {
    uint8_t val = cpu_bus_read(nes, nes->micro_addr);
    uint8_t dummy_x = nes->x + ~val + 1;

    set_flag(nes, STATUS_FLAG_NEGATIVE, dummy_x >> 7);
    set_flag(nes, STATUS_FLAG_CARRY, dummy_x <= nes->x);
    set_flag(nes, STATUS_FLAG_ZERO, dummy_x == 0);
}

void cpy(struct Nes* nes) {
    uint8_t val = cpu_bus_read(nes, nes->micro_addr);
    uint8_t dummy_y = nes->y + ~val + 1;

    set_flag(nes, STATUS_FLAG_NEGATIVE, dummy_y >> 7);
    set_flag(nes, STATUS_FLAG_CARRY, dummy_y <= nes->y);
    set_flag(nes, STATUS_FLAG_ZERO, dummy_y == 0);
}

void dec(struct Nes* nes) {
    uint8_t val = cpu_bus_read(nes, nes->micro_addr);
    val -= 1;
    cpu_bus_write(nes, nes->micro_addr, val);

    set_flag(nes, STATUS_FLAG_NEGATIVE, val >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, val == 0); 
}

void eor(struct Nes* nes) {
    nes->acc ^= cpu_bus_read(nes, nes->micro_addr);

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0); 
}

void clc(struct Nes* nes) {
    set_flag(nes, STATUS_FLAG_CARRY, 0);
}

void sec(struct Nes* nes) {
    set_flag(nes, STATUS_FLAG_CARRY, 1);
}

void cli(struct Nes* nes) {
    set_flag(nes, STATUS_FLAG_INTERRUPT, 0);
}

void sei(struct Nes* nes) {
    set_flag(nes, STATUS_FLAG_INTERRUPT, 1);
}

void clv(struct Nes* nes) {
    set_flag(nes, STATUS_FLAG_OVERFLOW, 0);
}

void cld(struct Nes* nes) {
    set_flag(nes, STATUS_FLAG_DECIMAL, 0);
}

void sed(struct Nes* nes) {
    set_flag(nes, STATUS_FLAG_DECIMAL, 1);
}

void inc(struct Nes* nes) {
    uint8_t val = cpu_bus_read(nes, nes->micro_addr);
    val += 1;
    cpu_bus_write(nes, nes->micro_addr, val);

    set_flag(nes, STATUS_FLAG_NEGATIVE, val >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, val == 0); 
}

void jmp(struct Nes* nes) {
    nes->pc = nes->micro_addr;
    if (nes->pc !=  0xC7E1) printf("jmp to 0x%04X\n", nes->pc);
}

void jsr(struct Nes* nes) {
    uint16_t addr = nes->pc - 1; //
    cpu_bus_write(nes, 0x0100 | nes->sp, (uint8_t)(addr >> 8));
    nes->sp -= 1;
    cpu_bus_write(nes, 0x0100 | nes->sp, (uint8_t)(addr));
    nes->sp -= 1;
    nes->pc = nes->micro_addr;
    if (nes->pc != 0xF4ED) printf("jsr to 0x%04X\n", nes->pc);
}

void lda(struct Nes* nes) {
    nes->acc = cpu_bus_read(nes, nes->micro_addr);
    
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
}

void ldx(struct Nes* nes) {
    nes->x = cpu_bus_read(nes, nes->micro_addr);
    
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->x >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->x == 0);
}

void ldy(struct Nes* nes) {
    nes->y = cpu_bus_read(nes, nes->micro_addr);
    
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->y >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->y == 0);
}

void lsr(struct Nes* nes) {
    uint8_t val = cpu_bus_read(nes, nes->micro_addr);
    uint8_t bit0 = val & 0x01;
    val >>= 1;
    cpu_bus_write(nes, nes->micro_addr, val);

    set_flag(nes, STATUS_FLAG_NEGATIVE, 0); // can this ever be negative?
    set_flag(nes, STATUS_FLAG_ZERO, val == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit0);
}

void lsr_imp(struct Nes* nes) {
    uint8_t bit0 = nes->acc & 0x01;
    nes->acc >>= 1;

    set_flag(nes, STATUS_FLAG_NEGATIVE, 0); // can this ever be negative?
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit0);
}

void nop(struct Nes* nes) {
    // leave blank
}

void ora(struct Nes* nes) {
    nes->acc |= cpu_bus_read(nes, nes->micro_addr);

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
}

void tax(struct Nes* nes) {
    nes->x = nes->acc;

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->x >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->x == 0);
}

void txa(struct Nes* nes) {
    nes->acc = nes->x;

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
}

void dex(struct Nes* nes) {
    nes->x -= 1;

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->x >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->x == 0);
}

void inx(struct Nes* nes) {
    nes->x += 1;

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->x >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->x == 0);
}

void tay(struct Nes* nes) {
    nes->y = nes->acc;

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->y >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->y == 0);
}

void tya(struct Nes* nes) {
    nes->acc = nes->y;

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
}

void dey(struct Nes* nes) {
    nes->y -= 1;

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->y >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->y == 0);
}

void iny(struct Nes* nes) {
    nes->y += 1;

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->y >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->y == 0);
}

void rol(struct Nes* nes) {
    uint8_t val = cpu_bus_read(nes, nes->micro_addr);
    uint8_t bit7 = val >> 7;
    val <<= 1;
    val |= get_flag(nes, STATUS_FLAG_CARRY); // bit0 = carry
    cpu_bus_write(nes, nes->micro_addr, val);

    set_flag(nes, STATUS_FLAG_NEGATIVE, val >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, val == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit7);
}

void rol_imp(struct Nes* nes) {
    uint8_t bit7 = nes->acc >> 7;
    nes->acc <<= 1;
    nes->acc |= get_flag(nes, STATUS_FLAG_CARRY); // bit0 = carry

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit7);
}

void ror(struct Nes* nes) {
    uint8_t val = cpu_bus_read(nes, nes->micro_addr);
    uint8_t bit0 = val & 0x01;
    val >>= 1;
    val |= get_flag(nes, STATUS_FLAG_CARRY) << 7; // bit7 = carry
    cpu_bus_write(nes, nes->micro_addr, val);

    set_flag(nes, STATUS_FLAG_NEGATIVE, val >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, val == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit0);
}

void ror_imp(struct Nes* nes) {
    uint8_t bit0 = nes->acc & 0x01;
    nes->acc >>= 1;
    nes->acc |= get_flag(nes, STATUS_FLAG_CARRY) << 7; // bit7 = carry

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit0);
}

// is:          10000111
// should be:   10100111
void rti(struct Nes* nes) {
    nes->sp += 1;
    nes->status = cpu_bus_read(nes, 0x0100 | nes->sp) | 0b00100000;
    nes->sp += 1;
    uint8_t lo = cpu_bus_read(nes, 0x0100 | nes->sp);
    nes->sp += 1;
    uint8_t hi = cpu_bus_read(nes, 0x0100 | nes->sp);
    nes->pc = make_u16(hi, lo);
    printf("rti to 0x%04X\n", nes->pc);
}

void rts(struct Nes* nes) {
    nes->sp += 1;
    uint8_t lo = cpu_bus_read(nes, 0x0100 | nes->sp);
    nes->sp += 1;
    uint8_t hi = cpu_bus_read(nes, 0x0100 | nes->sp);
    nes->pc = make_u16(hi, lo) + 1; // +1 ?
    if (nes->pc != 0xC7E4) printf("rts to 0x%04X\n", nes->pc);
}

void sbc(struct Nes* nes) {
    uint8_t old_acc = nes->acc;
    uint8_t val = ~cpu_bus_read(nes, nes->micro_addr);
    nes->acc += val + get_flag(nes, STATUS_FLAG_CARRY);

    uint8_t overflow = (val ^ old_acc) < 0x80 && (val ^ nes->acc) >> 7;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_OVERFLOW, overflow);
    set_flag(nes, STATUS_FLAG_CARRY, nes->acc <= old_acc);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
}

void sta(struct Nes* nes) {
    cpu_bus_write(nes, nes->micro_addr, nes->acc);
}

void txs(struct Nes* nes) {
    nes->sp = nes->x;
}

void tsx(struct Nes* nes) {
    nes->x = nes->sp;

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->x >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->x == 0);
}

void pha(struct Nes* nes) {
    cpu_bus_write(nes, 0x0100 | nes->sp, nes->acc);
    nes->sp -= 1;
}

void pla(struct Nes* nes) {
    nes->sp += 1;
    nes->acc = cpu_bus_read(nes, 0x0100 | nes->sp);

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
}

void php(struct Nes* nes) {
    cpu_bus_write(nes, 0x0100 | nes->sp, nes->status | 0b00110000);
    nes->sp -= 1;
}

void plp(struct Nes* nes) {
    nes->sp += 1;
    nes->status = cpu_bus_read(nes, 0x0100 | nes->sp) & 0b11101111 | 0b00100000;
}

void stx(struct Nes* nes) {
    cpu_bus_write(nes, nes->micro_addr, nes->x);
}

void sty(struct Nes* nes) {
    cpu_bus_write(nes, nes->micro_addr, nes->y);
}