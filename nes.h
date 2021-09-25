#include <stdint.h>

struct Nes {
	// CPU memory
	uint8_t cpu_ram[0x800];

	// CPU
	uint8_t acc;
	uint8_t x, y;
	uint16_t pc;
	uint8_t sp;
	uint8_t status;
	// CPU micro
	uint16_t micro_addr;
};

// init/free
void init_nes(struct Nes*);
void free_nes(struct Nes*);

// exec
int step(struct Nes*);

// read/write
// https://wiki.nesdev.org/w/index.php/CPU_memory_map
uint8_t cpu_read(struct Nes*, uint16_t);
void cpu_write(struct Nes*, uint16_t, uint8_t);

// as far as I'm aware, this is called for reach op
void fetch_op(struct Nes*);

// addressing modes (https://wiki.nesdev.org/w/index.php/CPU_addressing_modes)
void 	addrm_imp(struct Nes*); // ex. ROL A
void 	addrm_imm(struct Nes*); // ex. LDA #$32
void 	addrm_zp(struct Nes*);  // ex. LDA $32
void 	addrm_zpx(struct Nes*); // ex. LDA $32, X
void 	addrm_zpy(struct Nes*); // ex. LDA $32, Y
void 	addrm_abs(struct Nes*); // ex. LDA $3200
uint8_t addrm_abx(struct Nes*); // ex. LDA $3200, X
uint8_t addrm_aby(struct Nes*); // ex. LDA $3200, Y
void 	addrm_rel(struct Nes*); // ex. BNE symbol
void 	addrm_ind(struct Nes*); // ex. JMP ($3200)
void 	addrm_inx(struct Nes*); // ex. LDA ($32, X)
uint8_t addrm_iny(struct Nes*); // ex. LDA ($32), Y

// instructions (https://wiki.nesdev.org/w/index.php/6502_instructions)
void lda(struct Nes*);
void sta(struct Nes*);
void stx(struct Nes*);
void sty(struct Nes*);
// ..