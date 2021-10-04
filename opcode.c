#include "nes.h"

void adc(struct Nes* nes) {
    uint8_t old_acc = nes->acc;
    uint8_t val = cpu_read(nes, nes->micro_addr);
    nes->acc += val + (nes->status & (1 << STATUS_FLAG_CARRY) > 0);

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_OVERFLOW, old_acc >> 7 ^ nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_CARRY, nes->acc < old_acc);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
}

void and(struct Nes* nes) {
    nes->acc &= cpu_read(nes, nes->micro_addr);

    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
}

void asl(struct Nes* nes) {
    uint8_t val = cpu_read(nes, nes->micro_addr);
    uint8_t bit7 = val >> 7;
    val <<= 1;
    cpu_write(nes, nes->micro_addr, val);

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
    uint8_t val = cpu_read(nes, nes->micro_addr);
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

void cmp(struct Nes* nes) {
    // TODO: implement
}

void cpx(struct Nes* nes) {
    // TODO: implement
}

void cpy(struct Nes* nes) {
    // TODO: implement
}

void dec(struct Nes* nes) {
    uint8_t val = cpu_read(nes, nes->micro_addr);
    val -= 1;
    cpu_write(nes, nes->micro_addr, val);

    set_flag(nes, STATUS_FLAG_NEGATIVE, val >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, val == 0); 
}

void eor(struct Nes* nes) {
    nes->acc ^= cpu_read(nes, nes->micro_addr);

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
    uint8_t val = cpu_read(nes, nes->micro_addr);
    val += 1;
    cpu_write(nes, nes->micro_addr, val);

    set_flag(nes, STATUS_FLAG_NEGATIVE, val >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, val == 0); 
}

void jmp(struct Nes* nes) {
    nes->pc = nes->micro_addr;
}

void jsr(struct Nes* nes) {
    cpu_write(nes, 0x0100 | nes->sp, (uint8_t)(nes->pc >> 8));
    nes->sp -= 1;
    cpu_write(nes, 0x0100 | nes->sp, (uint8_t)(nes->pc));
    nes->sp -= 1;
    nes->pc = nes->micro_addr;
}

void lda(struct Nes* nes) {
    nes->acc = cpu_read(nes, nes->micro_addr);
    
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
}

void ldx(struct Nes* nes) {
    nes->x = cpu_read(nes, nes->micro_addr);
    
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->x >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->x == 0);
}

void ldy(struct Nes* nes) {
    nes->y = cpu_read(nes, nes->micro_addr);
    
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->y >> 7);
    set_flag(nes, STATUS_FLAG_ZERO, nes->y == 0);
}

void lsr(struct Nes* nes) {
    uint8_t val = cpu_read(nes, nes->micro_addr);
    uint8_t bit0 = val & 0x01;
    val >>= 1;
    cpu_write(nes, nes->micro_addr, val);

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
    nes->acc |= cpu_read(nes, nes->micro_addr);

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
    uint8_t val = cpu_read(nes, nes->micro_addr);
    uint8_t bit7 = val >> 7;
    val <<= 1;
    val |= get_flag(nes, STATUS_FLAG_CARRY); // bit0 = carry
    cpu_write(nes, nes->micro_addr, val);

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
    uint8_t val = cpu_read(nes, nes->micro_addr);
    uint8_t bit0 = val & 0x01;
    val >>= 1;
    val |= get_flag(nes, STATUS_FLAG_CARRY) << 7; // bit7 = carry
    cpu_write(nes, nes->micro_addr, val);

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

void rti(struct Nes* nes) {
    // TODO: implement
}

void rts(struct Nes* nes) {
    nes->sp += 1;
    uint8_t lo = cpu_read(nes, 0x0100 | nes->sp);
    nes->sp += 1;
    uint8_t hi = cpu_read(nes, 0x0100 | nes->sp);
    nes->pc = make_u16(hi, lo);
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

void sta(struct Nes* nes) {
    cpu_write(nes, nes->micro_addr, nes->acc);
}

void txs(struct Nes* nes) {
    nes->sp = nes->x;
}

void tsx(struct Nes* nes) {
    nes->x = nes->sp;
}

void pha(struct Nes* nes) {
    cpu_write(nes, 0x0100 | nes->sp, nes->acc);
    nes->sp -= 1;
}

void pla(struct Nes* nes) {
    nes->sp += 1;
    nes->acc = cpu_read(nes, 0x0100 | nes->sp);
}

void php(struct Nes* nes) {
    cpu_write(nes, 0x0100 | nes->sp, nes->status);
    nes->sp -= 1;
}

void plp(struct Nes* nes) {
    nes->sp += 1;
    nes->status = cpu_read(nes, 0x0100 | nes->sp);
}

void stx(struct Nes* nes) {
    cpu_write(nes, nes->micro_addr, nes->x);
}

void sty(struct Nes* nes) {
    cpu_write(nes, nes->micro_addr, nes->y);
}