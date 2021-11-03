#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "cartridge.h"
#include "mapper0.h"
#include "error.h"

uint16_t make_u16(uint8_t hi, uint8_t lo);

int load_cartridge_from_file(char* filename, struct Cartridge* cartridge) {
    int err = 0;
    uint8_t header[16] = { 0 };
    uint8_t* data = NULL;
    FILE* fp = NULL;

    // validate filename and cartridge
    if (filename == NULL) return error(INVALID_INPUT, "Null filename");
    if (cartridge == NULL) return error(INVALID_INPUT, "Null cartridge output");

    // open filename as readonly binary
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        err = error(IO_ERROR, "Could not open file \"%s\"", filename);
        goto close;
    }

    // get file size
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // alloc rom (subtract 16 bytes for header, which is not part of the rom)
    data = malloc(size - 16);
    if (data == 0) {
        err = error(ALLOC_ERROR, "Could not allocate %i bytes", size - 16);
        goto close;
    }

    // read first 16 bytes as header, and the rest as rom
    fread(header, 1, 16, fp);
    fread(data, 1, size - 16, fp);

    nlog("ROM file successfully loaded\n  Filename: %s\n  size: %i", filename, size - 16);

    // init cartridge from data parts
    err = load_cartridge_from_data(header, data, cartridge);
    if (err > 0) goto close; // yes, I know, this line is a no-op

    // close all
close:
    free(data);
    fclose(fp);
    return err;
}

int load_cartridge_from_data(uint8_t header[16], uint8_t* data, struct Cartridge* cartridge) {
    int err = 0;
    
    // verify iNES 1.0
    uint8_t ines1 = header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1A;
    if (!ines1) return error(ROM_FORMAT_ERROR, "INES 1.0 header not found");

    // verify iNES 2.0 (note: iNES 2.0 currently unsupported, fail if detected)
    uint8_t ines2 = header[7] & 0b1100 == 0b1000;
    if (ines2) return error(ROM_FORMAT_ERROR, "INES 2.0 header found, but not supported");

    // extract data from header, assuming ines 1.0
    uint32_t prg_rom_size = header[4] * 0x4000; // prg rom size (in units of 0x4000 bytes)
    uint32_t chr_rom_size = header[5] * 0x2000; // chr rom size (in units of 0x2000 bytes)
    uint8_t mapper = (header[7] & 0x11110000) | (header[6] >> 4); // mapper byte ID
    uint8_t mirroring = header[6] & 0b1; // ciram mirroring mode

    // nlog rom data
    nlog("ROM parsed\n  Header info:\n    mapper: %i\n    prg size: 0x%X\n    chr size: 0x%X\n    mirroring: %i (0 = horizontal, 1 = vertical)", mapper, prg_rom_size, chr_rom_size, mirroring);

    // create mapper (for now, only mapper0 is supported)
    switch (mapper) {
        case 0: {
            // these first checks technically wrong? NROM boards are mapper 0, but not all mapper 0 are NROM?
            if (prg_rom_size != 0x4000 && prg_rom_size != 0x8000) return error(ROM_FORMAT_ERROR, "PRG ROM must be 0x4000 or 0x8000 bytes (value was 0x%04X)", prg_rom_size);
            if (chr_rom_size != 0x2000) return error(ROM_FORMAT_ERROR, "CHR ROM must be 0x2000 bytes (value was 0x%04X)", chr_rom_size);

            // allocate and initialize mapper 0
            struct Mapper0* mapper0 = malloc(sizeof(struct Mapper0));
            memcpy(&mapper0->prg_rom, data, prg_rom_size); // copy prg rom
            memcpy(&mapper0->chr_rom, data + prg_rom_size, chr_rom_size); // copy chr rom
            mapper0->mask = prg_rom_size - 1; // prg rom wrapping mask
            mapper0->mirroring = mirroring; // ciram mirroring (currently unused)

            // initialize cartridge wrapper
            cartridge->data = mapper0;
            cartridge->prg_read = (void*)mapper0_prg_read;
            cartridge->prg_write = (void*)mapper0_prg_write;
            cartridge->chr_read = (void*)mapper0_chr_read;
        } break;
        default: return error(ROM_FORMAT_ERROR, "Unsupported mapper (value was %i)", mapper);
    }

    // for diagnostic purposes, read and print the interrupt vectors
    uint16_t nmi_addr = 0xFFFA;
    uint16_t reset_addr = 0xFFFC;
    uint16_t irq_addr = 0xFFFE;
    uint16_t nmi, reset, irq;
    uint8_t hi, lo;
    lo = cartridge_prg_read(cartridge, nmi_addr);
    hi = cartridge_prg_read(cartridge, nmi_addr + 1);
    nmi = make_u16(hi, lo);
    lo = cartridge_prg_read(cartridge, reset_addr);
    hi = cartridge_prg_read(cartridge, reset_addr + 1);
    reset = make_u16(hi, lo);
    lo = cartridge_prg_read(cartridge, irq_addr);
    hi = cartridge_prg_read(cartridge, irq_addr + 1);
    irq = make_u16(hi, lo);
    nlog("Interrupt vectors:\n  Vectors:\n    nmi: 0x%04X\n    reset: 0x%04X\n    irq: 0x%04X", nmi, reset, irq);
}

void free_cartridge(struct Cartridge* cartridge) {
    free(cartridge->data);
}

uint8_t cartridge_prg_read(struct Cartridge* cartridge, uint16_t addr) {
    return (cartridge->prg_read)(cartridge->data, addr);
}

uint8_t cartridge_chr_read(struct Cartridge* cartridge, uint16_t addr) {
    return (cartridge->chr_read)(cartridge->data, addr);
}

void cartridge_prg_write(struct Cartridge* cartridge, uint16_t addr, uint8_t val) {
    (cartridge->prg_write)(cartridge->data, addr, val);
}
