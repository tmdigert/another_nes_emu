#pragma once

#include <stdint.h>
#include "nes.h"

void draw_ppu_nametables(struct Nes* nes, uint8_t pixels[4 * SCREEN_WIDTH * SCREEN_HEIGHT]);