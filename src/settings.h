#pragma once

#include <SDL2/SDL.h>

struct Settings {
	// keybinds
	SDL_Scancode a;
	SDL_Scancode b;
	SDL_Scancode start;
	SDL_Scancode select;
	SDL_Scancode up;
	SDL_Scancode down;
	SDL_Scancode left;
	SDL_Scancode right;
};

void settings_load_from_file(struct Settings*, const char*);
void settings_create_from_default(struct Settings*);
void settings_write_to_file(struct Settings*, const char*);