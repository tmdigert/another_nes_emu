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

void step(struct Nes* nes) {
	// fetch first opcode, result in Nes::micro_instr
	uint8_t op = fetch_op(nes);

	//
	uint8_t cycles = 0xFF; //
	switch (op) {
		case 0x85: cycles = (addrm_zp(nes); sta(nes); 3); break;
		case 0x95: cycles = (addrm_zpx(nes); sta(nes); 4); break;
		case 0x8D: cycles = (addrm_abs(nes); sta(nes); 4); break;
		case 0x9D: cycles = (addrm_abx(nes); sta(nes); 5); break;
		case 0x99: cycles = (addrm_aby(nes); sta(nes); 5); break;
		case 0x81: cycles = (addrm_inx(nes); sta(nes); 6); break;
		case 0x91: cycles = (addrm_iny(nes); sta(nes); 6); break;	
		case 0x86: cycles = (addrm_zp(nes); stx(nes); 3); break;
		case 0x96: cycles = (addrm_zpy(nes); stx(nes); 4); break;
		case 0x8E: cycles = (addrm_abs(nes); stx(nes); 4); break;	
		case 0x84: cycles = (addrm_zp(nes); sty(nes); 3); break;
		case 0x94: cycles = (addrm_zpx(nes); sty(nes); 4); break;
		case 0x8C: cycles = (addrm_abs(nes); sty(nes); 4); break;	
		case 0xA9: cycles = (addrm_imm(nes); lda(nes); 2); break;
		case 0xA5: cycles = (addrm_zp(nes); lda(nes); 3); break;
		case 0xB5: cycles = (addrm_zpx(nes); lda(nes); 4); break;
		case 0xAD: cycles = (addrm_abs(nes); lda(nes); 4); break;
		case 0xBD: cycles = (uint8_t oops = addrm_abx(nes); lda(nes); 4 + oops); break;
		case 0xB9: cycles = (uint8_t oops = addrm_abx(nes); lda(nes); 4 + oops); break;
		case 0xA1: cycles = (addrm_inx(nes); lda(nes); 6); break;
		case 0xB1: cycles = (uint8_t oops = addrm_iny(nes); lda(nes); 5 + oops); break;
		default:
		// unhandled opcode error
		break;
	}
}

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
	addr += x;
	// store internal address
	nes->micro_addr = (uint16_t)addr;
}

void addrm_zpy(struct Nes* nes) {
	// read operand (zp address)
	uint8_t addr = (uint16_t)cpu_read(nes, nes->pc);
	nes->pc += 1;
	// inc zp address (by y)
	addr += y;
	// store internal address
	nes->micro_addr = (uint16_t)addr;
}

void addrm_inx(struct Nes* nes) {
	// read operand (zp address)
	uint8_t addr = cpu_read(nes, nes->pc);
	nes->pc += 1;
	// inc zp address (by x)
	addr += x;
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
	uint8_t carry = lo + new->x < lo;
	lo += new->y;
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
	uint8_t carry = lo + new->x < lo;
	lo += new->x;
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
	uint8_t carry = lo + new->y < lo;
	lo += new->y;
	hi += carry;
	// store internal address
	nes->micro_addr = make_u16(hi, lo);
	return carry;
}

void lda(struct Nes* nes) {
	nes->acc = cpu_read(nes, nes->micro_addr);
}

void sta(struct Nes* nes){
	nes->micro_addr = nes->acc;
}

