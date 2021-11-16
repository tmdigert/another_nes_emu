#include <stdint.h>
#include "cartridge.h"

#define STATUS_FLAG_CARRY       0
#define STATUS_FLAG_ZERO        1
#define STATUS_FLAG_INTERRUPT   2
#define STATUS_FLAG_DECIMAL     3
#define STATUS_FLAG_BREAK       4
#define STATUS_FLAG_ALWAYS      5
#define STATUS_FLAG_OVERFLOW    6
#define STATUS_FLAG_NEGATIVE    7

struct Nes {
    // Cartridge
    struct Cartridge cartridge;

    // CPU
    uint8_t ram[0x0800];
    uint8_t acc;
    uint8_t x, y;
    uint16_t pc;
    uint8_t sp;
    uint8_t status;
    // interrupt 
    uint8_t reset;
    uint8_t nmi;
    // CPU micro
    uint16_t micro_addr;

    // PPU
    uint8_t ciram[0x0800];
    uint8_t oam[0x100];
    uint8_t palette[0x20];
    uint32_t cycle;
    uint8_t ppuctrl; // ppu register @ 0x2000
    uint8_t ppumask; // ppu register @ 0x2001
    uint8_t ppustatus; // ppu register @ 0x2002
    uint8_t oamaddr; // ppu register @ 0x2003
    uint16_t ppuscroll; // ppu register @ 0x2005
    uint16_t ppuaddr; // ppu register @ 0x2006
};

uint16_t make_u16(uint8_t hi, uint8_t lo);

// init/free
void init_nes(struct Nes*, struct Cartridge);
void free_nes(struct Nes*);

// exec
uint8_t step_cpu(struct Nes*);
uint8_t step_ppu(struct Nes*, uint8_t);
void reset(struct Nes*);

// misc
void set_flag(struct Nes*, uint8_t, uint8_t);
uint8_t get_flag(struct Nes*, uint8_t);

// bus
// https://wiki.nesdev.org/w/index.php/CPU_memory_map
uint8_t cpu_bus_read(struct Nes*, uint16_t);
void cpu_bus_write(struct Nes*, uint16_t, uint8_t);
// https://wiki.nesdev.org/w/index.php/PPU_memory_map
uint8_t ppu_bus_read(struct Nes*, uint16_t);
void ppu_bus_write(struct Nes*, uint16_t, uint8_t);

// addressing modes (https://wiki.nesdev.org/w/index.php/CPU_addressing_modes)
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
void    adc(struct Nes*);
void    and(struct Nes*);
void    asl(struct Nes*);
void    asl_imp(struct Nes*);
uint8_t bcc(struct Nes*);
uint8_t bcs(struct Nes*);
uint8_t beq(struct Nes*);
void    bit(struct Nes*);
uint8_t bmi(struct Nes*);
uint8_t bne(struct Nes*);
uint8_t bpl(struct Nes*);
void    brk(struct Nes*);
uint8_t bvc(struct Nes*);
uint8_t bvs(struct Nes*);
void    clc(struct Nes*);
void    cld(struct Nes*);
void    cli(struct Nes*);
void    clv(struct Nes*);
void    cmp(struct Nes*);
void    cpx(struct Nes*);
void    cpy(struct Nes*);
void    dec(struct Nes*);
void    dex(struct Nes*);
void    dey(struct Nes*);
void    eor(struct Nes*);
void    inc(struct Nes*);
void    inx(struct Nes*);
void    iny(struct Nes*);
void    jmp(struct Nes*);
void    jsr(struct Nes*);
void    lda(struct Nes*);
void    ldx(struct Nes*);
void    ldy(struct Nes*);
void    lsr(struct Nes*);
void    lsr_imp(struct Nes*);
void    nop(struct Nes*);
void    ora(struct Nes*);
void    pha(struct Nes*);
void    php(struct Nes*);
void    pla(struct Nes*);
void    plp(struct Nes*);
void    rol(struct Nes*);
void    rol_imp(struct Nes*);
void    ror(struct Nes*);
void    ror_imp(struct Nes*);
void    rti(struct Nes*);
void    rts(struct Nes*);
void    sbc(struct Nes*);
void    sec(struct Nes*);
void    sed(struct Nes*);
void    sei(struct Nes*);
void    sta(struct Nes*);
void    stx(struct Nes*);
void    sty(struct Nes*);
void    tax(struct Nes*);
void    tay(struct Nes*);
void    tsx(struct Nes*);
void    txa(struct Nes*);
void    txs(struct Nes*);
void    tya(struct Nes*);
// ..