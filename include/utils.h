#ifndef CRCLONE_UTILS_H
#define CRCLONE_UTILS_H

#include <stdbool.h>
#include <SDL2/SDL_render.h>

char* strdup_(char const*, bool);
char* strcatdyn(char*, char const*, bool);
char* remove_last_utf8_char_dyn(char*);
size_t get_utf8_length(char const*);
void destroy_SDL_Texture(SDL_Texture*);
SDL_Texture* create_SDL_texture_from_BMP(SDL_Renderer*, char const*);

#endif