#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cartridge.h"

int load_cartridge_from_file(char* filename, struct Cartridge* cartridge) {
	printf("loading ROM: \"%s\".\n", filename);
	int err = 0;
	uint8_t header[16];
	uint8_t* data = 0;
	FILE* fp = 0;

	// open file
    fp = fopen(filename, "rb");
    if (fp == 0) goto unsuccessful_file_load;

    // get file size
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    printf("  File size: %i bytes (+ 16 byte header).\n", size - 16);

    // alloc rom
    data = malloc(size - 16);

    // read
    fread(header, 1, 16, fp);
    fread(data, 1, size - 16, fp);

    // init cartridge
   	err = load_cartridge_from_data(header, data, cartridge);
    if (err == -1) goto unsuccessful_rom_load;

    printf("ROM load success.\n", filename);
    goto close;

    // rom load unsuccessful
unsuccessful_rom_load:
    printf("ROM load unsuccessful.\n");
    err = -1;
    goto close;

    // file load unsuccessful
unsuccessful_file_load:
	printf("File load unsuccessful.\n");
	err = -1;
	goto close;

    // close all
close:
    free(data);
    fclose(fp);
    return err;
}

int load_cartridge_from_data(uint8_t header[16], uint8_t* data, struct Cartridge* cartridge) {
	uint8_t ines1 = header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1A;
	if (!ines1) {
		printf("  !ROM is not formatted correctly!\n");
		return -1;
	}
	printf("  iNES header detected.\n");

	// iNES 2.0 format 
	uint8_t ines2 = header[7] & 0b1100 == 0b1000;
	if (ines2) {
		printf("  !iNES 2.0 headers unsupported!");
		return -1;
	}

	// assume iNES
	uint32_t prg_rom_size = header[4] * 0x4000;
	uint32_t chr_rom_size = header[5] * 0x2000;
	uint8_t mapper = (header[7] & 0x11110000) | (header[6] >> 4);
	printf("  Header info:\n    mapper: %i\n    prg size: 0x%X\n    chr size: 0x%X\n", mapper, prg_rom_size, chr_rom_size);

	// create mapper
	switch (mapper) {
		case 0: {
			struct Mapper0* mapper0 = malloc(sizeof(struct Mapper0));
			// copy prg and chr rom into cartridge
			memcpy(&mapper0->prg_rom, data, prg_rom_size);
			memcpy(&mapper0->chr_rom, data + prg_rom_size, chr_rom_size);
			// set mirroring
			mapper0->mask = prg_rom_size - 1;
			// initialize cartridge
			cartridge->data = mapper0;
			cartridge->mapper = 0;
			cartridge->prg_read = (void*)mapper0_prg_read;
			cartridge->prg_write = (void*)mapper0_prg_write;
			cartridge->chr_read = (void*)mapper0_chr_read;
		} break;
		default: {
			printf("  !Unsupported mapper!\n");
			return -1;
		} break;
	}
}

void free_cartridge(struct Cartridge* cartridge) {
	free(cartridge->data);
}