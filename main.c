#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>

#include "nes.h"

/*int load_rom(char* filename, uint8_t header[16], uint8_t** rom) {
    FILE* fp = fopen(filename, "r");
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    printf("Rom loaded with size %i\n", size);
    *rom = malloc(size - 16);
    fseek (fp , 0, SEEK_SET);
    fread(header, 1, 16, fp);
    fseek (fp , 16 , SEEK_SET);
    fread(*rom, 1, size - 16, fp);
    fclose(fp);
}

int main(int argc, char* argv[]) {
    // load ROM
    uint8_t header[16];
    uint8_t* rom = 0;
    load_rom("nestest.nes", header, &rom);

    // generate nes
    struct Nes nes;
    init_nes(&nes, rom);
    nes.pc = 0xC000;

    // step the nes 7 times (complete first brk)
    int cyc = 7;
    for (int instr = 1; instr <= 6000; instr++) {
        printf("%6i PC:%04X A:%02X X:%02X Y:%02X P:%02X, SP:%02X CYC:%i\n", instr, nes.pc, nes.acc, nes.x, nes.y, nes.status, nes.sp, cyc);
        fflush(stdout);
        cyc += step(&nes);
    }

    free_nes(&nes);
}*/

int main(int argc, char* argv[]) {
    // load ROM
    struct Cartridge cartridge;
    load_cartridge_from_file("donkeykong.nes", &cartridge);
    free_cartridge(&cartridge);
}