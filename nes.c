#include "nes.h"

uint16_t make_u16(uint8_t hi, uint8_t lo) {
	return ((uint16_t)hi << 8) | (uint16_t)lo;
}

int test_run(struct Nes* const uint8_t* instructions);

/*uint8_t cpu_read(struct Nes*, uint16_t addr) {
	// range 0..0x1FFF -> internal ram
	if (addr <= 0x1FFF) {
		return cpu_ram[addr & 0x7FF];
	}
	// range 0x2000..0x3FFF -> PPU
	if (addr <= 0x3FFF) {
		return ppu_read(nes, addr & 0x07);
	}
	// range 0x4000..0x4017 -> APU/IO
	if (addr <= 0x4017) {
		return apu_read(nes, addr & 0x17);
	}
	// range 0x4018..0x401F -> test mode (ignore?)
	if (addr <= 0x401F) {
		return 0;
	}
	// range 0x4020..0xFFFF -> cartridge
	return cartridge_read(nes->cartridge, addr);
}

void cpu_write(struct Nes*, uint16_t addr, uint8_t byte) {

}*/

void fetch_op(struct Nes* nes) {
	nes->micro_instr = cpu_read(nes, nes->pc);
	nes->pc += 1;
}

void addrm_imp(struct Nes* nes) {
	// this one kind of sucks
}

void addrm_imm(struct Nes* nes) {
	nes->micro_addr = nes->pc;
	nes->pc += 1;
}

void addrm_zp(struct Nes* nes) {
	nes->micro_addr = (uint16_t)cpu_read(nes, nes->pc);
	nes->pc += 1;
}

void addrm_zpx(struct Nes* nes) {
	nes->micro_addr = (uint16_t)cpu_read(nes, nes->pc);
	nes->pc += 1;
	nes->micro_addr += x;
}

void addrm_zpy(struct Nes* nes) {
	nes->micro_addr = (uint16_t)cpu_read(nes, nes->pc);
	nes->pc += 1;
	nes->micro_addr += y;
}

void addrm_inx(struct Nes* nes) {
	uint8_t addr = cpu_read(nes, nes->pc);
	nes->pc += 1;
	addr += x; // wrapping
	uint8_t lo = cpu_read(nes, addr);
	uint8_t hi = cpu_read(nes, addr + 1);
	nes->micro_addr = make_u16(hi, lo);
}

void addrm_iny(struct Nes* nes) {
	uint8_t addr = cpu_read(nes, nes->pc);
	nes->pc += 1;
	uint8_t hi = cpu_read(nes, addr);
	uint8_t lo = cpu_read(nes, addr + 1);
	uint8_t carry = lo + new->x < lo;
	lo += new->y;
	hi += carry;
	nes->micro_addr = make_u16(hi, lo);
}

void addrm_abs(struct Nes* nes) {
	uint8_t lo = cpu_read(nes, nes->pc);
	nes->pc += 1;
	uint8_t hi = cpu_read(nes, nes->pc);
	nes->pc += 1;
	nes->micro_addr = make_u16(hi, lo);
}

void addrm_abx(struct Nes* nes) {
	uint8_t lo = cpu_read(nes, nes->pc);
	nes->pc += 1;
	uint8_t hi = cpu_read(nes, nes->pc);
	nes->pc += 1;
	uint8_t carry = lo + new->x < lo;
	lo += new->x;
	hi += carry;
	nes->micro_addr = make_u16(hi, lo);
}

void addrm_aby(struct Nes* nes) {
	uint8_t lo = cpu_read(nes, nes->pc);
	nes->pc += 1;
	uint8_t hi = cpu_read(nes, nes->pc);
	nes->pc += 1;
	uint8_t carry = lo + new->y < lo;
	lo += new->y;
	hi += carry;
	nes->micro_addr = make_u16(hi, lo);
}

void lda(struct Nes* nes) {
	nes->acc = cpu_read(nes, nes->micro_addr);
}

