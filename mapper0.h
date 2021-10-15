#include <stdint.h>

// specific mapper
struct Mapper0 {
	uint8_t prg_rom[0x8000];
	uint16_t mask; // for mirroring
	uint8_t chr_rom[0x2000];
};

uint8_t mapper0_prg_read(struct Mapper0*, uint16_t);
uint8_t mapper0_chr_read(struct Mapper0*, uint16_t);
void mapper0_prg_write(struct Mapper0*, uint16_t, uint8_t);