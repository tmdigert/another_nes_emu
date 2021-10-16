#include <assert.h>
#include "nes.h"

void step_ppu(struct Nes* nes, uint8_t cycles) {

}

uint8_t ppu_read(struct Nes* nes, uint16_t addr) {
	assert((0x2000 <= addr && addr < 0x2008) || addr == 0x4014);
	switch (addr) {
		// PPUDATA: rw
		0x2007: {

		} break;

		default: {

		} break;
	}
}

void ppu_write(struct Nes* nes, uint16_t addr, uint8_t byte) {
	assert((0x2000 <= addr && addr < 0x2008) || addr == 0x4014);
	switch (addr) {
		// PPUADDR: w
		0x2006: {
			nes->ppuaddr <<= 8;
			nes->ppuaddr &= byte;
		} break;
	}
}