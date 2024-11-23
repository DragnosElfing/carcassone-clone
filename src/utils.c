#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wchar.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>

#include "utils.h"

#ifdef _CRCLONE_DEBUG
    #include "debug/debugmalloc.h"
#endif

char* strdup_(char const* str, bool is_utf8)
{
    size_t len = strlen(str) * (is_utf8 ? sizeof(wchar_t) : sizeof(char));
    char* dyn_str = malloc(len + 1);
    if(dyn_str == NULL) return NULL;
    
    strcpy(dyn_str, str);

    return dyn_str;
}

char* strcatdyn(char* original, char const* to_cat, bool is_utf8)
{
    if(original == NULL) return NULL;
    
    size_t len = (strlen(original) + strlen(to_cat)) * (is_utf8 ? sizeof(wchar_t) : sizeof(char));
    size_t cat_from = strlen(original);

    char* new_str = malloc(len + 1);
    if(new_str == NULL) return NULL;
    
    strcpy(new_str, original);
    strcpy(new_str + cat_from, to_cat);
    free(original);
    original = NULL;

    return new_str;
}

void remove_last_utf8_char_dyn(char* str)
{
    //
}

void destroy_SDL_Texture(SDL_Texture* texture)
{
    if(texture == NULL) return;

    SDL_DestroyTexture(texture);
    texture = NULL;
}

SDL_Texture* create_SDL_texture_from_BMP(SDL_Renderer* renderer, char const* source_path)
{
    SDL_Surface* src_surface = SDL_LoadBMP(source_path);
    if(src_surface == NULL) return NULL;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, src_surface);
    SDL_FreeSurface(src_surface);

    return texture;
}