#include <assert.h>
#include <stdio.h>
#include "nes.h"

uint8_t cpu_bus_read(struct Nes* nes, uint16_t addr) {
    // ram [0x0000, 0x1FFF]
    if (addr <= 0x1FFF) {
    	addr &= 0x07FF;
    	return nes->ram[addr];
    }

    // ppu [0x2000, 0x3FFF]
    if (addr <= 0x3FFF) {
    	switch (0x2000 | (addr & 0b111)) {
    		case 0x2002: {
    			uint8_t out = nes->ppustatus;
    			nes->ppustatus &= 0x7F;
    			return out;
    		} break;
    		case 0x2007: return ppu_bus_read(nes, nes->ppuaddr);
    		default: {
				printf("Unimplemented CPU bus read: 0x%04X\n at 0x%04X\n", addr, nes->pc);
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
				nes->ppuctrl |= byte;
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
				nes->ppuaddr = (nes->ppuaddr << 8) | byte;
				nes->ppuaddr += (nes->ppuctrl & 0b0100 > 0) * 31 + 1;
			} break;
			case 0x2007: {
				ppu_bus_write(nes, nes->ppuaddr, byte); 
				nes->ppuaddr += (nes->ppuctrl & 0b0100 > 0) * 31 + 1;
			} break;
			default: {
				printf("Unimplemented CPU bus write: 0x%04X at 0x%04X with value 0x%02X\n", addr, nes->pc, byte);
				assert(0);
			} break;
		}
		return;
	}

	// APU and IO [0x4000, 0x4017]
    if (addr <= 0x4017) {
    	// TODO: implement
    	return;
    }
	//Pulse 1
    if(addr == 0x4000){
                uint8_t duty_val = (byte>> 6);
                uint8_t env_val = ((byte<< 2) >> 7);
                uint8_t vol_val = ((byte << 3) >> 7);
                uint8_t vol_env_val = ((byte << 4) >> 4);

                nes->pulse1_duty = duty_val;
                nes->pulse1_env = env_val;
                nes->pulse1_vol = vol_val;
                nes->pulse1_vol_env = vol_env_val;
		return;
        }

        if(addr == 0x4001){
                uint8_t sweep_val = (byte >> 7);
                uint8_t period_val = ((byte << 1) >> 5);
                uint8_t negate_val = ((byte << 4) >> 7);
                uint8_t shift_val = ((byte << 4) >> 4);

                nes->pulse1_sweep = duty_val;
                nes->pulse1_env = env_val;
                nes->pulse1_negate = vol_val;
                nes->pulse1_shift = vol_env_val;
		return;
        }
        if(addr == 0x4002){
                uint8_t timer_val = (byte);
                
                nes-> pulse1_timer = timer_val;
                return;
        }
        if(addr == 0x4003){
                unit8_t len_val = (byte >> 3);
                unit8_t timer_hi_val = ((byte << 4) >> 4;
                nes->pulse1_len = len_val;
                nes->pulse1_timer_hi = timer_hi_val;
                return;
        }
        
        //triangle
        if(addr == 0x4008){
                nes->tri_lin=(byte >> 7);
                nes->tri_load=(byte << 1) >> 1;
        }
        if(addr == 0x400A){
                nes->tri_timer = byte;
        }
        if(addr == 0x400B){
                nes->tri_len = (byte >> 3);
                nes->tri_timer_hi = byte >>3;
        }

    	return;
    }
    //Pulse 2
    if(addr == 0x4004){
                uint8_t duty_val = (nes->ram[addr] >> 6);
                uint8_t env_val = ((nes->ram[addr]<< 2) >> 7);
                uint8_t vol_val = (nes->ram[addr] << 3) >> 7);
                uint8_t vol_env_val = (nes->ram[addr] << 4) >> 4);

                nes->pulse1_duty = duty_val;
                nes->pulse1_env = env_val;
                nes->pulse1_vol = vol_val;
                nes->pulse1_vol_env = vol_env_val;
		return;
        }

        if(addr == 0x4005){
                uint8_t sweep_val = (nes->ram[addr] >> 7);
                uint8_t period_val = ((nes->ram[addr] << 1) >> 5);
                uint8_t negate_val = (nes->ram[addr] << 4) >> 7);
                uint8_t shift_val = (nes->ram[addr] << 4) >> 4);

                nes->pulse2_sweep = duty_val;
                nes->pulse2_env = env_val;
                nes->pulse2_negate = vol_val;
                nes->pulse2_shift = vol_env_val;
		return;
        }
        if(addr == 0x4006){
                uint8_t timer_val = (byte);
                
                nes-> pulse2_timer = timer_val;
                return;
        }
        if(addr == 0x4007){
                unit8_t len_val = (byte >> 3;
                unit8_t timer_hi_val = (byte << 4) >> 4;
                nes->pulse2_len = len_val;
                nes->pulse2_timer_hi = timer_hi_val;
                return;
        }
        
        //triangle
        if(addr == 0x4008){
                nes->tri_lin=(nes->ram[addr] >> 7);
                nes->tri_load=(nes->ram[addr] << 1) >> 1;
        }
        if(addr == 0x400A){
                nes->tri_timer = nes->ram[addr];
        }
        if(addr == 0x400B){
                nes->tri_len = (nes->ram[addr] >> 3);
                nes->tri_timer_hi = (nes->ram[addr) >>3;
        }
        if(addr == 0x4015){
	        nes->enb_pulse1 = (nes-ram[addr] << 7 ) >> 7;
	        nes->enb_pulse2 = (nes-ram[addr] << 6 ) >> 7;
	        nes->enb_try = (nes-ram[addr] << 5 ) >> 7;
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

		// unreachable?
		assert(0);
	}

	// palette [0x3F00, 0x3FFF]
	if (addr <= 0x3FFF) {
		return nes->palette[(addr - 0x3F00) & 0x1F];
	}

	// nothing exists beyond 0x3FFF
	return 0;
}

void ppu_bus_write(struct Nes* nes, uint16_t addr, uint8_t byte) {
	// chr rom [0x0000, 0x1FFF]
	if (addr <= 0x1FFF) {
		return; // TODO: some cartridges do have writable chr
		//return cartridge_chr_read(&nes->cartridge, addr);
	}

	// ciram [0x2000, 0x3EFF]
	if (addr <= 0x3EFF) {
		// clip to 0x2000
		addr = 0x2000 | (addr & 0xFFF);

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

		// unreachable?
		assert(0);
	}

	// palette [0x3F00, 0x3FFF]
	if (addr <= 0x3FFF) {
		nes->palette[(addr - 0x3F00) & 0x1F] = byte;
		return;
	}

	// nothing exists beyond 0x3FFF
	return;
}
