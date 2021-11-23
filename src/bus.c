#include <assert.h>
#include <stdio.h>

#include "nes.h"
#include "error.h"

uint8_t cpu_bus_read(struct Nes* nes, uint16_t addr) {
    // ram [0x0000, 0x1FFF]
    if (addr <= 0x1FFF) {
        return nes->ram[addr & 0x07FF];
    }

    // ppu [0x2000, 0x3FFF]
    if (addr <= 0x3FFF) {
        switch (0x2000 | (addr & 0b111)) {
            // ppustatus
            case 0x2002: {
                uint8_t out = nes->ppustatus;
                nes->ppustatus &= 0x7F;
                nes->ppuaddr = 0; // is this correct?
                return out;
            }
            // ppuadata
            case 0x2007: {
                uint8_t out = nes->read_buffer;
                nes->read_buffer = ppu_bus_read(nes, nes->ppuaddr);
                nes->ppuaddr += ((nes->ppuctrl & 0b0100) > 0) * 31 + 1;
                // return read_buffer immediately if in palette range
                return ((nes->ppuaddr & 0x3FFF) < 0x3F00) ? out : nes->read_buffer;
            }
            //
            default: {
                return 0x7F;
                error(UNIMPLEMENTED, "Unimplemented bus read: 0x%04X", 0x2000 | (addr & 0b111));
                assert(0);
            }
        }
    }

    // APU and IO [0x4000, 0x4017]
    if (addr <= 0x4017) {
        switch (addr) {
            // joypad 1
            case 0x4016: {
                uint8_t out = nes->joy1 & 0x01;
                nes->joy1 >>= 1;
                return out;
            }
            // joypad 2
            case 0x4017: {
                uint8_t out = nes->joy2 & 0x01;
                nes->joy2 >>= 1;
                return out;
            }
            // do not abort on unhandled APU
            default: {
                return 0;
            }
        }
    }

    // normally disabled, don't implement
    if (addr <= 0x401F) {
        return 0;
    }

    // cart [0x4020, 0xFFFF]
    return cartridge_prg_read(&nes->cartridge, addr);
}

void cpu_bus_write(struct Nes* nes, uint16_t addr, uint8_t byte) { 
    // ram [0x0000, 0x1FFF]
    if (addr <= 0x1FFF) {
        nes->ram[addr & 0x07FF] = byte;
        return;
    }

    // ppu [0x2000, 0x3FFF]
    if (addr <= 0x3FFF) {
        // TODO: most of these are probably wrong
        switch (0x2000 | (addr & 0b111)) {
            // ppuctrl
            case 0x2000: {
                nes->ppuctrl = byte;
                return;
            }
            // ppumask
            case 0x2001: {
                nes->ppumask = byte;
                return;
            }
            // oamaddr
            case 0x2003: {
                nes->oamaddr = byte;
                return;
            }
            // ppuscroll
            case 0x2005: {
                nes->ppuscroll <<= 8;
                nes->ppuscroll |= byte;
                return;
            }
            // ppuaddr
            case 0x2006: {
                nes->ppuaddr <<= 8;
                nes->ppuaddr |= byte;
                return;
            }
            // ppudata
            case 0x2007: {
                ppu_bus_write(nes, nes->ppuaddr, byte); 
                nes->ppuaddr += ((nes->ppuctrl & 0b0100) > 0) * 31 + 1;
                return;
            }
            //
            default: {
                error(UNIMPLEMENTED, "Unimplemented bus write: 0x%04X <- 0x%02X", addr, byte);
                assert(0);
                return;
            }
        }
    }

    // APU and IO [0x4000, 0x4017]
    if (addr <= 0x4017) {
        // TODO: implement
        switch (addr) {
            // input polling
            case 0x4016: {
                // write 1 or 0?
                if (byte == 1) {
                    nes->joy1 = nes->input1;
                    nes->joy2 = nes->input2;
                }
                return;
            };
            // OAM
            case 0x4014: {
                uint8_t i = nes->oamaddr;
                uint8_t i_start = i;
                do {
                    nes->oam[i] = cpu_bus_read(nes, (byte << 8) + i);
                    i++;
                } while (i != i_start);
                nes->oam_delay = 1;
                return;
            };
            // do not abort on unhandled APU
            default: {
                return;
            }
        }
    }

    // normally disabled, don't implement
    if (addr <= 0x401F) {
        return;
    }

    // cart [0x4020, 0xFFFF]
    cartridge_prg_write(&nes->cartridge, addr, byte);
    return;
}

uint8_t ppu_bus_read(struct Nes* nes, uint16_t addr) {
    // ppu addresses above 0x3FFF are mirrored
    addr &= 0x3FFF;

    // chr rom [0x0000, 0x1FFF]
    if (addr <= 0x1FFF) {
        return cartridge_chr_read(&nes->cartridge, addr);
    }

    // ciram [0x2000, 0x3EFF]
    if (addr <= 0x3EFF) {
        // horizontal
        /*uint16_t ciram_mask =  0b001111111111;
        uint16_t mirror_mask = 0b100000000000;
        addr = ((addr & mirror_mask) >> 1) | (addr & ciram_mask);*/

        // vertical
        uint16_t ciram_mask =  0b001111111111;
        uint16_t mirror_mask = 0b010000000000;
        addr = addr & (mirror_mask | ciram_mask);

        // 
        return nes->ciram[addr];
    }

    // palette [0x3F00, 0x3FFF]
    if (addr <= 0x3FFF) {
        if (addr % 4 == 0) return nes->palette[addr & 0x0F];
        else return nes->palette[addr & 0x1F];
    }

    // unreachable, abort
    error(UNREACHABLE, "This line should not be reachable");
    assert(0);
}

void ppu_bus_write(struct Nes* nes, uint16_t addr, uint8_t byte) {
    // ppu addresses above 0x3FFF are mirrored
    addr &= 0x3FFF;

    // chr rom [0x0000, 0x1FFF]
    if (addr <= 0x1FFF) {
        return; // TODO: some cartridges do have writable chr
        //return cartridge_chr_read(&nes->cartridge, addr);
    }

    // ciram [0x2000, 0x3EFF]
    if (addr <= 0x3EFF) {
        // horizontal
        /*uint16_t ciram_mask =  0b001111111111;
        uint16_t mirror_mask = 0b100000000000;
        addr = ((addr & mirror_mask) >> 1) | (addr & ciram_mask);*/

        // vertical
        uint16_t ciram_mask =  0b001111111111;
        uint16_t mirror_mask = 0b010000000000;
        addr = addr & (mirror_mask | ciram_mask);

        nes->ciram[addr] = byte;
        return;
    }

    // palette [0x3F00, 0x3FFF]
    if (addr <= 0x3FFF) {
        //nlog("ppu_bus_write: 0x%04X (pc: 0x%04X) with value 0x%02X", addr, nes->pc, byte);
        if (addr % 4 == 0) nes->palette[addr & 0x0F] = byte;
        else nes->palette[addr & 0x1F] = byte;
        /*if (addr % 4 == 0 && addr) nes->palette[0] = byte;
        else nes->palette[addr & 0x1F] = byte;*/
        return;
    }

    // unreachable, abort
    error(UNREACHABLE, "This line should not be reachable");
    assert(0);
}