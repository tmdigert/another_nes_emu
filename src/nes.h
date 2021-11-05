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
    uint8_t oam[0xFF];
    uint8_t palette[0x20];
    uint32_t cycle;
    uint8_t ppuaddr_latch;
    uint8_t ppuscroll_latch;
    uint8_t ppuctrl; // ppu register @ 0x2000
    uint8_t ppumask; // ppu register @ 0x2001
    uint8_t ppustatus; // ppu register @ 0x2002
    uint8_t oamaddr; // ppu register @ 0x2003
    uint16_t ppuscroll; // ppu register @ 0x2005
    uint16_t ppuaddr; // ppu register @ 0x2006
    
     //APU
    uint8_t apu_read; //set if system needs to update sound

    //Pulse1
    uint8_t pulse1_duty;
    uint8_t pulse1_env;
    uint8_t pulse1_vol;
    uint8_t pulse1_vol_env;
    uint8_t pulse1_sweep;
    uint8_t pulse1_period;
    uint8_t pulse1_negate;
    uint8_t pulse1_shift;
    uint8_t pulse1_timer;
    uint8_t pulse1_len;
    uint8_t pulse1_timer_hi;

    //Pulse2
    uint8_t pulse2_duty;
    uint8_t pulse2_env;
    uint8_t pulse2_vol;
    uint8_t pulse2_vol_env;
    uint8_t pulse2_sweep;
    uint8_t pulse2_period;
    uint8_t pulse2_negate;
    uint8_t pulse2_shift;
    uint8_t pulse2_timer;
    uint8_t pulse2_len;
    uint8_t pulse2_timer_hi;

    //Triangle
    uint8_t tri_lin;
    uint8_t tri_load;
    uint8_t tri_timer;
    uint8_t tri_len;
    uint8_t tri_timer_hi;

    //noise
    uint8_t noise_env;
    uint8_t noise_vol;
    uint8_t noise_vol_env;
    uint8_t noise_loop;
    uint8_t noise_period;
    uint8_t noise_load;

    //DMC
    uint8_t dmc_irq;
    uint8_t dmc_loop;
    uint8_t dmc_freq;
    uint8_t dmc_counter;
    uint8_t dmc_addr;
    uint8_t dmc_len;

    //status
    uint8_t enb_dmc;
    uint8_t enb_noise;
    uint8_t enb_tri;
    uint8_t enb_pulse1;
    uint8_t enb_pulse2;

    uint8_t intr_noise;
    uint8_t intr_tri;
    uint8_t intr_pulse1;
    uint8_t intr_pulse2;
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
