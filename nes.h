#include <stdint.h>

#define STATUS_FLAG_CARRY       0
#define STATUS_FLAG_ZERO        1
#define STATUS_FLAG_INTERRUPT   2
#define STATUS_FLAG_DECIMAL     3
#define STATUS_FLAG_BREAK       4
#define STATUS_FLAG_ALWAYS      5
#define STATUS_FLAG_OVERFLOW    6
#define STATUS_FLAG_NEGATIVE    7

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
uint8_t step(struct Nes*);

// misc
void set_flag(struct Nes*, uint8_t, uint8_t);

// read/write
// https://wiki.nesdev.org/w/index.php/CPU_memory_map
uint8_t cpu_read(struct Nes*, uint16_t);
void cpu_write(struct Nes*, uint16_t, uint8_t);

// as far as I'm aware, this is called for reach op
uint8_t fetch_op(struct Nes*);

// addressing modes (https://wiki.nesdev.org/w/index.php/CPU_addressing_modes)
void    addrm_imp(struct Nes*); // ex. ROL A
void    addrm_imm(struct Nes*); // ex. LDA #$32
void    addrm_zp(struct Nes*);  // ex. LDA $32
void    addrm_zpx(struct Nes*); // ex. LDA $32, X
void    addrm_zpy(struct Nes*); // ex. LDA $32, Y
void    addrm_abs(struct Nes*); // ex. LDA $3200
uint8_t addrm_abx(struct Nes*); // ex. LDA $3200, X
uint8_t addrm_aby(struct Nes*); // ex. LDA $3200, Y
void    addrm_rel(struct Nes*); // ex. BNE symbol
void    addrm_ind(struct Nes*); // ex. JMP ($3200)
void    addrm_inx(struct Nes*); // ex. LDA ($32, X)
uint8_t addrm_iny(struct Nes*); // ex. LDA ($32), Y

// instructions (https://wiki.nesdev.org/w/index.php/6502_instructions)
void lda(struct Nes*);
void sta(struct Nes*);
void stx(struct Nes*);
void sty(struct Nes*);
// ..