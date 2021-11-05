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
            case 0x2002: {
                uint8_t out = nes->ppustatus;
                nes->ppustatus &= 0x7F;
                nes->ppuaddr_latch = 0;
                nes->ppuscroll_latch = 0;
                return out;
            } break;
            case 0x2007: {
                uint8_t out = ppu_bus_read(nes, nes->ppuaddr);
                nes->ppuaddr += ((nes->ppuctrl & 0b0100) > 0) * 31 + 1;
                return out;
            } break;
            default: {
                error(UNIMPLEMENTED, "Unimplemented PPU reg read: 0x%04X", addr);
                assert(0);
            } break;
        }
    }

    // APU and IO [0x4000, 0x4017]
    if (addr <= 0x4017) {
        // TODO: implement
        return 0;
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
            case 0x2000: {
                nes->ppuctrl = byte;
                assert((nes->ppuctrl & 0b11) == 0);
            } break;
            case 0x2001: {
                nes->ppumask |= byte;
            } break;
            case 0x2003: {
                // hopefully we can ignore this for now
            } break;
            case 0x2005: {
                nes->ppuscroll <<= 8;
                nes->ppuscroll |= byte;
            } break;
            case 0x2006: {
                nes->ppuaddr <<= 8;
                nes->ppuaddr |= byte;
            } break;
            case 0x2007: {
                ppu_bus_write(nes, nes->ppuaddr, byte); 
                nes->ppuaddr += ((nes->ppuctrl & 0b0100) > 0) * 31 + 1;
            } break;
            default: {
                error(UNIMPLEMENTED, "Unimplemented PPU reg write: 0x%04X", addr);
                assert(0);
            } break;
        }
        return;
    }

    // APU and IO [0x4000, 0x4017]
    if (addr <= 0x4017) {
        // TODO: implement
        switch(0x4000){
            case 0x4000: {
                nes->pulse_1_1 = byte;
            }break;
            case 0x4001: {
                nes->pulse_1_2 = byte;
            }break;
            case 0x4002: {
                nes->pulse_1_3 = byte;
            }break;
            case 0x4003: {
                nes->pulse_1_4 = byte;
            }break;
            case 0x4004: {
                nes->pulse_2_1 = byte;
            }break;
            case 0x4005: {
                nes->pulse_2_2 = byte;
            }break;
            case 0x4006: {
                nes->pulse_2_3 = byte;
            }break;
            case 0x4007: {
                nes->pulse_2_4 = byte;
            }break;
            case 0x4008: {
                nes->tri_1 = byte;
            }break;
            case 0x400A: {
                nes->tri_2 = byte;
            }break;
            case 0x400B: {
                nes->tri_3 = byte;
            }break;
            default: {
                error(UNIMPLEMENTED, "Unimplemented APU reg write: 0x%04X", addr);
                assert(0);
            } 
            break;
            
        }
        return;
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
        // clip to 0x2000
        addr = 0x2000 | (addr & 0xFFF);

        // TODO: this assumes horizontal mirroring, should probably be implemented by the cartridge
        if (addr < 0x2400) {
            return nes->ciram[addr - 0x2000];
        }
        // in section B
        if (addr < 0x2800) {
            return nes->ciram[addr - 0x2400];
        }
        // in section C
        if (addr < 0x2C00) {
            return nes->ciram[addr - 0x2400];
        }
        // in section D
        if (addr < 0x3000) {
            return nes->ciram[addr - 0x2800];
        }

        // unreachable, abort
        error(UNREACHABLE, "This line should not be reachable");
        assert(0);
    }

    // palette [0x3F00, 0x3FFF]
    if (addr <= 0x3FFF) {
        if (addr % 4 == 0) return nes->palette[0];
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
        // clip to 0x2000
        addr = 0x2000 | addr & 0xFFF;

        // TODO: this assumes horizontal mirroring, should probably be implemented by the cartridge
        if (addr < 0x2400) {
            nes->ciram[addr - 0x2000] = byte;
            return;
        }
        // in section B
        if (addr < 0x2800) {
            nes->ciram[addr - 0x2400] = byte;
            return;
        }
        // in section C
        if (addr < 0x2C00) {
            nes->ciram[addr - 0x2400] = byte;
            return;
        }
        // in section D
        if (addr < 0x3000) {
            nes->ciram[addr - 0x2800] = byte;
            return;
        }

        // unreachable, abort
        error(UNREACHABLE, "This line should not be reachable");
        assert(0);
    }

    // palette [0x3F00, 0x3FFF]
    if (addr <= 0x3FFF) {
        //nlog("ppu_bus_write: 0x%04X (pc: 0x%04X) with value 0x%02X", addr, nes->pc, byte);
        if (addr % 4 == 0) nes->palette[0] = byte;
        else nes->palette[addr & 0x1F] = byte;
        return;
    }

    // unreachable, abort
    error(UNREACHABLE, "This line should not be reachable");
    assert(0);
}
