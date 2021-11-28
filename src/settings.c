#include "settings.h"

void settings_create_from_default(struct Settings* settings) {
	settings->a = SDL_SCANCODE_Z;
	settings->b = SDL_SCANCODE_X;
	settings->start = SDL_SCANCODE_RETURN;
	settings->select = SDL_SCANCODE_RSHIFT;
	settings->up = SDL_SCANCODE_UP;
	settings->down = SDL_SCANCODE_DOWN;
	settings->left = SDL_SCANCODE_LEFT;
	settings->right = SDL_SCANCODE_RIGHT;
}