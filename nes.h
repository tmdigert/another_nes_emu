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
	uint8_t micro_instr;
	uint16_t micro_addr;
};

int test_run(struct Nes*, const uint8_t*);

int step(struct Nes*);

// read/write
// https://wiki.nesdev.org/w/index.php/CPU_memory_map
uint8_t cpu_read(struct Nes*, uint16_t);
void cpu_write(struct Nes*, uint16_t, uint8_t);

// as far as I'm aware, these two are called for each instruction
void fetch_op(struct Nes*);
void fetch_addr(struct Nes*); // maybe not this one?

// addressing modes (https://wiki.nesdev.org/w/index.php/CPU_addressing_modes)
void addrm_imp(struct Nes*);
void addrm_imm(struct Nes*); // LDA #$32 (Const operand)
void addrm_zp(struct Nes*);  // LDA $32
void addrm_zpx(struct Nes*); // LDA $32, X
void addrm_zpy(struct Nes*); // LDA $32, Y
void addrm_abs(struct Nes*); // LDA $3200
void addrm_abx(struct Nes*); // LDA $3200, X
void addrm_aby(struct Nes*); // LDA $3200, Y
void addrm_rel(struct Nes*); // BNE symbol
void addrm_ind(struct Nes*); // JMP ($3200)
void addrm_inx(struct Nes*); // LDA ($32, X)
void addrm_iny(struct Nes*); // LDA ($32), Y

// instructions (https://wiki.nesdev.org/w/index.php/6502_instructions)
void lda(struct Nes*);
// ..