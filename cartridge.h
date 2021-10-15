#include <stdint.h>

struct Cartridge {
	void* data;
	uint8_t mapper;

	uint8_t (*prg_read)(void*, uint16_t);
	uint8_t (*chr_read)(void*, uint16_t);
	void (*prg_write)(void*, uint16_t, uint8_t);
};

int load_cartridge_from_file(char*, struct Cartridge*);
int load_cartridge_from_data(uint8_t*, uint8_t*, struct Cartridge*);
void free_cartridge(struct Cartridge*);

uint8_t cartridge_prg_read(struct Cartridge*, uint16_t);
uint8_t cartridge_chr_read(struct Cartridge*, uint16_t);
void cartridge_prg_write(struct Cartridge*, uint16_t, uint8_t);

// specific mapper
struct Mapper0 {
	uint8_t prg_rom[0x8000];
	uint16_t mask; // for mirroring
	uint8_t chr_rom[0x2000];
};

uint8_t mapper0_prg_read(struct Mapper0*, uint16_t);
uint8_t mapper0_chr_read(struct Mapper0*, uint16_t);
void mapper0_prg_write(struct Mapper0*, uint16_t, uint8_t);