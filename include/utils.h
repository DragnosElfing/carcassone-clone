#ifndef CRCLONE_UTILS_H
#define CRCLONE_UTILS_H

#include <stdbool.h>
#include <SDL2/SDL_render.h>

// Pár segédfüggvény.

size_t mb_strlen(char const*);
char* strdup_(char const*);
char* strcatdyn(char*, char const*);
char* mb_remove_last_char_dyn(char*);
void destroy_SDL_Texture(SDL_Texture*);
SDL_Texture* create_SDL_texture_from_BMP(SDL_Renderer*, char const*);

#endif