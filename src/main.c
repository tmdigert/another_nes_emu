#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "nes.h"
#include "getopt.c"
#include "settings.h"

#include <assert.h>
#include "mapper0.h"
#include "debug.h"

int main(int argc, char** argv) {
    //
    SDL_Delay(400);

    // arg parsing
    char* filename = NULL;
    int opt = -1;
    while ((opt = getopt(argc, argv, "f:h")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                break;
            case 'h':
                printf("  -f [arg_name]  --  Specifies rom filename as arg_name.\n");
                printf("  -h             --  Print command line arguments.\n");
                break;
        }
    }

    if (filename == NULL) return -1;

    // load ROM
    struct Cartridge cartridge;
    if (load_cartridge_from_file(filename, &cartridge) > 0) return -1;

    // load nes
    struct Nes nes;
    init_nes(&nes, cartridge);
    
    uint8_t apu_len_lookup[] = {10, 16, 20, 5, 40, 5, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14, 12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30};
    
    // Render
    uint8_t lookup[] = {
        84, 84, 84, 0, 30, 116, 8, 16, 144, 48, 0, 136, 68, 0, 100, 92, 0, 48, 84, 4, 0, 60, 24, 0, 32, 42, 0, 8, 58, 0, 0, 64, 0, 0, 60, 0, 0, 50, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        152, 150, 152, 8, 76, 196, 48, 50, 236, 92, 30, 228, 136, 20, 176, 160, 20, 100, 152, 34, 32, 120, 60, 0, 84, 90, 0, 40, 114, 0, 8, 124, 0, 0, 118, 40, 0, 102, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        236, 238, 236, 76, 154, 236, 120, 124, 236, 176, 98, 236, 228, 84, 236, 236, 88, 180, 236, 106, 100, 212, 136, 32, 160, 170, 0, 116, 196, 0, 76, 208, 32, 56, 204, 108, 56, 180, 204, 60, 60, 60, 0, 0, 0, 0, 0, 0, 
        236, 238, 236, 168, 204, 236, 188, 188, 236, 212, 178, 236, 236, 174, 236, 236, 174, 212, 236, 180, 176, 228, 196, 144, 204, 210, 120, 180, 222, 120, 168, 226, 144, 152, 226, 180, 160, 214, 228, 160, 162, 160, 0, 0, 0, 0, 0, 0
    };

    struct Settings settings;
    settings_load_from_file(&settings, "settings.conf");

    // init SDL video
    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);

    // create windows
    SDL_Window* window = SDL_CreateWindow("NES Emu", 1920/2, 1080/2, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    assert(window);
    SDL_Window* nametable_window = SDL_CreateWindow("NES Emu: Nametable", 2 * SCREEN_WIDTH, 2 * SCREEN_HEIGHT, 2 * SCREEN_WIDTH, 2 * SCREEN_HEIGHT, 0);
    assert(nametable_window);

    // allocate pixel space
    uint8_t* screen_pixels = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 3);
    uint8_t* nes_pixels = malloc(SCREEN_WIDTH * SCREEN_HEIGHT);
    
    uint8_t p1_halt_flg;
    uint8_t p1_vol_flg;
    uint8_t p1_length_ctr;
    uint8_t p1_duty;
    uint8_t p1_timer_lo;
    uint8_t p1_timer_hi;
    uint8_t p1_vol_val;
    uint8_t p1_len;
    uint8_t p1_stat;

    uint8_t p2_halt_flg;
    uint8_t p2_vol_flg;
    uint8_t p2_length_ctr;
    uint8_t p2_duty;
    uint8_t p2_timer_lo;
    uint8_t p2_timer_hi;
    uint8_t p2_vol_val;
    uint8_t p2_len;
    uint8_t p2_stat;

    uint8_t tri_lin_ctl;
    uint8_t tri_lin_ld;
    uint8_t tri_lo;
    uint8_t tri_len_ld;
    uint8_t tri_hi;
    uint8_t tri_stat;

    uint8_t p1_count = 0;
    uint8_t p2_count = 0;
    uint8_t tri_count = 0;

    uint8_t frame_mode;

    uint8_t tri_count_halt = 0;

    Mix_Chunk *pulse_master = Mix_LoadWAV("square_256hz_no_duty.wav");

    Mix_Chunk *p1_duty_125 = Mix_LoadWAV("square_256hz_no_duty.wav");
    Mix_Chunk *p1_duty_25 = Mix_LoadWAV("square_256hz_no_duty.wav");
    Mix_Chunk *p1_duty_50 = Mix_LoadWAV("square_256hz_no_duty.wav");
    Mix_Chunk *p1_duty_25n = Mix_LoadWAV("square_256hz_no_duty.wav");

    Mix_Chunk *p2_duty_125 = Mix_LoadWAV("square_256hz_no_duty.wav");
    Mix_Chunk *p2_duty_25 = Mix_LoadWAV("square_256hz_no_duty.wav");
    Mix_Chunk *p2_duty_50 = Mix_LoadWAV("square_256hz_no_duty.wav");
    Mix_Chunk *p2_duty_25n = Mix_LoadWAV("square_256hz_no_duty.wav");

    Mix_Chunk *tri_master = Mix_LoadWAV("tri_128hz_short.wav");
    Uint8* tri_mstr_buf = tri_master -> abuf;
  	Uint32 tri_mstr_len = tri_master -> alen;


    // run n vblanks of rom
    int acc_cycle = 7;
    int vblanks = 0;

    uint8_t next = 0;
    while (1) {
        int start = SDL_GetTicks();
        int cycle = step_cpu(&nes);

        if (step_ppu(&nes, 3 * cycle, nes_pixels)) {
            vblanks += 1;
            p1_stat = (nes.apu_status) & 0b1;
            p2_stat = (nes.apu_status >> 1) & 0b1;
            tri_stat = (nes.apu_status >> 2) & 0b1;
            if(tri_stat == 0b0){
              nlog("stop tri");
              Mix_HaltChannel(2);
              //tri_master -> abuf = tri_mstr_buf;
              //tri_master -> alen = tri_mstr_len;
              //nes.update_tri = 0;
              //nes.apu_read = 0;
            }
            if(p1_stat == 0b0){
              nlog("stop p1");
              Mix_HaltChannel(0);
            }
            if(p2_stat == 0b0){
              nlog("stop p2");
              Mix_HaltChannel(1);
            }


            if(tri_count_halt == 0){ //decrement counter
              if(tri_count == 0){
                Mix_HaltChannel(2);
              }
              else{
                tri_count -= 1;
              }

            }
            else{
              nlog("tri count halt");
            }
            if(p1_halt_flg == 0){ //decrement counter
              if(p1_count == 0){
                Mix_HaltChannel(0);
              }
              else{
                p1_count -= 1;
              }

            }
            else{
              nlog("p1 count halt");
            }
            if(p2_halt_flg == 0){ //decrement counter
              if(p2_count == 0){
                Mix_HaltChannel(0);
              }
              else{
                p2_count -= 1;
              }

            }
            else{
              nlog("p2 count halt");
            }


                        if(nes.apu_read == 1){

                          nes.apu_read = 0;

                          p1_duty = (nes.pulse_1_1 >> 6) & 0b11;
                          p1_halt_flg = (nes.pulse_1_1 >> 5) & 1;
                          p1_vol_flg = (nes.pulse_1_1 >> 4) & 1;
                          p1_vol_val = (nes.pulse_1_1) & 0b1111;

                          p1_timer_lo = nes.pulse_1_3;

                          p1_timer_hi = nes.pulse_1_4 & 0b111;
                          p1_length_ctr = (nes.pulse_1_4 >> 3) & 0b11111;

                          p2_duty = (nes.pulse_2_1 >> 6) & 0b11;
                          p2_halt_flg = (nes.pulse_2_1 >> 5) & 1;
                          p2_vol_flg = (nes.pulse_2_1 >> 4) & 1;
                          p2_vol_val = (nes.pulse_2_1) & 0b1111;

                          p2_timer_lo = nes.pulse_2_3;

                          p2_timer_hi = nes.pulse_2_4 & 0b111;
                          p2_length_ctr = (nes.pulse_2_4 >> 3) & 0b11111;


                          tri_lin_ctl = (nes.tri_1 >> 7) & 1;
                          tri_lin_ld = (nes.tri_1) & 0b1111111;

                          tri_lo = nes.tri_2;
                          tri_len_ld = nes.tri_3 >> 3;
                          tri_hi = nes.tri_3 & 0b111;

                          frame_mode = (nes.frame_count >> 7) & 0b1;

                          //if(nes.apu_read == 1){

                            //nlog("%i\n", tri_stat);
                          //}

                          //if(SDL_GetTicks()%1000 == 0){
                            //nlog("%i\n", tri_stat);
                          //}

                          if(p1_stat == 0b1 && nes.update_p1){

                            if(Mix_Playing(0) == 1){
                              Mix_HaltChannel(0);
                            }

                          p1_count = apu_len_lookup[p1_length_ctr] / 5;

                          int cur_vol = (p1_vol_val) * 8;

                          Mix_PlayChannel(0, pulse_master, -1);
                          Mix_Volume(0, cur_vol);

                          if(p1_vol_flg == 1){
                            Mix_FadeOutChannel(0, (int)(p1_count * 5));
                          }


                          nes.apu_read = 0;
                          nes.update_p1 = 0;

                          }
                          if(p2_stat == 0b1 && nes.update_p2){

                            if(Mix_Playing(1) == 1){
                              Mix_HaltChannel(1);
                          }

                          p2_count = apu_len_lookup[p2_length_ctr] / 5;

                          int cur_vol = (p2_vol_val) * 8;

                          Mix_PlayChannel(1, pulse_master, -1);
                          Mix_Volume(1, cur_vol);

                          if(p2_vol_flg == 1){
                            Mix_FadeOutChannel(1, (int)(p2_count * 5));
                          }


                          nes.apu_read = 0;
                          nes.update_p1 = 0;
                          }

                          if(tri_stat == 0b1 && nes.update_tri == 1){
                            if(Mix_Playing(2) == 1){
                              Mix_HaltChannel(2);
                              //tri_master -> abuf = tri_mstr_buf;
                          		//tri_master -> alen = tri_mstr_len;
                            }


                            int tri_timer = (int)(tri_lo + (tri_hi * 0b100000000));
                            int tri_freq = (1789773/(16*(tri_timer + 1)));
                            float note_ratio = ((float)tri_freq/(float)128);
                            int new_sample = 44100;
                            //if(tri_freq < 10000){
                              new_sample = (int)(note_ratio * 44100);
                            //}
                            //int tri_count = (int)(4*60*tri_freq);
                            //int tri_timer_int = checked((int)tri_timer_int);

                            uint16_t frm_dat;
                          	int chan_dat;

                          	Mix_QuerySpec(NULL, &frm_dat, &chan_dat);
                            nlog("length val: %i\n", apu_len_lookup[tri_len_ld]);
                            uint8_t len_cand = apu_len_lookup[tri_len_ld] / 5;
                            /*
                            SDL_AudioCVT new_tone;

                            SDL_BuildAudioCVT(&new_tone, frm_dat, chan_dat, new_sample, frm_dat, chan_dat, 44100);

                            new_tone.buf = (uint8_t*)SDL_malloc(new_tone.len*new_tone.len_mult);
                            new_tone.len = tri_master->alen;

                            SDL_ConvertAudio(&new_tone);
                            SDL_memcpy(new_tone.buf, tri_master->abuf, tri_master->alen);
                            */
                            //tri_master -> abuf = new_tone.buf;
                        		//tri_master -> alen = new_tone.len_cvt;

                            uint8_t lin_cand = tri_lin_ld / 5;
                            if(len_cand >= lin_cand){
                              tri_count = len_cand;
                            }
                            else{
                              tri_count = lin_cand;
                            }

                            //nlog("tri len: ")

                            if(tri_lin_ctl == 0){
                              tri_count_halt = 0;

                            }
                            else{
                              tri_count_halt = 1;
                            }

                            //nlog("%i\n", tri_lin_ld);
                            nlog("freq: %i\n", tri_freq);
                            nlog("sample: %i\n", new_sample);
                            //nlog("tri_ctl: %i\n", tri_lin_ctl);
                            /*
                            if(nes.tri_1 == 0){
                               int n = 1;
                            }
                            else{
                              if(tri_lin_ctl == 0){
                                tri_count = (tri_len_ld/4);
                              }
                              else{
                                int n = 1;
                              }

                            }*/
                            nlog("Length: %i\n", tri_count);
                            //if(frame_mode == 1){
                           //  nlog("frame mode!");
                            //}
                            //nlog("Play!");
                            Mix_PlayChannel(2, tri_master, -1);
                            nlog("Done!");
                            //tri_stat = 0;
                            nes.update_tri = 0;

                            nes.apu_read = 0;
                          }


                        }
            // debug render nametable
            uint8_t nametable_pixels[SCREEN_WIDTH * SCREEN_HEIGHT * 4];
            draw_ppu_nametables(&nes, nametable_pixels);
            uint8_t nametable_rgb[SCREEN_WIDTH * SCREEN_HEIGHT * 4 * 3];
            for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 4; i++) {
                nametable_rgb[3 * i + 0] = lookup[3 * nametable_pixels[i] + 0];
                nametable_rgb[3 * i + 1] = lookup[3 * nametable_pixels[i] + 1];
                nametable_rgb[3 * i + 2] = lookup[3 * nametable_pixels[i] + 2];
            }
            SDL_Surface* temp = SDL_CreateRGBSurfaceFrom(nametable_rgb, 2 * SCREEN_WIDTH, 2 * SCREEN_HEIGHT, 24, 3 * 2 * SCREEN_WIDTH, 0x0000FF, 0x00FF00, 0xFF0000, 0);
            SDL_BlitSurface(temp, 0, SDL_GetWindowSurface(nametable_window), 0);
            SDL_UpdateWindowSurface(nametable_window); 
            SDL_FreeSurface(temp);

            // TEMP
            draw_oam_sprites(&nes, nes_pixels);

            // keyboard
            int keys = 0;
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_WINDOWEVENT) {
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_MOVED: {
                            nlog("moved");
                        } break;
                        case SDL_WINDOWEVENT_CLOSE: {
                            goto exit;
                        } break;
                    }
                }
            }
            const uint8_t* keyboard = SDL_GetKeyboardState(&keys);
            uint8_t state = 0;
            state |= keyboard[settings.right] << 7;
            state |= keyboard[settings.left] << 6;
            state |= keyboard[settings.down] << 5;
            state |= keyboard[settings.up] << 4;
            state |= keyboard[settings.start] << 3;
            state |= keyboard[settings.select] << 2;
            state |= keyboard[settings.b] << 1;
            state |= keyboard[settings.a] << 0;
            nes.input1 = state;

            // convert NES pixel to RGB pixels
            for (int y = 0; y < SCREEN_HEIGHT; y++) {
                for (int x = 0; x < SCREEN_WIDTH; x++) {
                    uint32_t dst_i = x + y * SCREEN_WIDTH;
                    uint32_t src_i = x + y * SCREEN_WIDTH;
                    screen_pixels[dst_i * 3 + 0] = lookup[nes_pixels[src_i] * 3 + 0];
                    screen_pixels[dst_i * 3 + 1] = lookup[nes_pixels[src_i] * 3 + 1];
                    screen_pixels[dst_i * 3 + 2] = lookup[nes_pixels[src_i] * 3 + 2];
                }
            }     

            // blit and render (vsync?)
            SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(screen_pixels, 256, 240, 24, 256 * 3, 0x0000FF, 0x00FF00, 0xFF0000, 0);
            assert(surface);
            SDL_BlitSurface(surface, 0, SDL_GetWindowSurface(window), 0);
            SDL_UpdateWindowSurface(window); 
            SDL_FreeSurface(surface);

            int end = SDL_GetTicks();
            if (end - start < 16) {
                SDL_Delay(16 - (end - start));
            }
        };
        acc_cycle += cycle;
    }

exit:
    
    Mix_FreeChunk(pulse_master);
    Mix_FreeChunk(p1_duty_125);
    Mix_FreeChunk(p1_duty_25);
    Mix_FreeChunk(p1_duty_50);
    Mix_FreeChunk(p1_duty_25n);

    Mix_FreeChunk(p2_duty_125);
    Mix_FreeChunk(p2_duty_25);
    Mix_FreeChunk(p2_duty_50);
    Mix_FreeChunk(p2_duty_25n);

    Mix_FreeChunk(tri_master);

    //
    settings_write_to_file(&settings, "settings.conf");

    // free render
    SDL_DestroyWindow(window);
    SDL_DestroyWindow(nametable_window);
    free(screen_pixels);
    free(nes_pixels);

    // free nes
    free_nes(&nes);
}
