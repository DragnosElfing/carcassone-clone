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

char* remove_last_utf8_char_dyn(char* str)
{
    if(str == NULL) return NULL;
    
    size_t len = strlen(str);
    if (len == 0) return NULL;

    // UTF-8 enkódolási szabályok:
    // - 1-byte karakter: 0xxxxxxx
    // - 2-byte karakter: 110xxxxx 10xxxxxx
    // - 3-byte karakter: 1110xxxx 10xxxxxx 10xxxxxx
    // - 4-byte karakter: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

    size_t p = len - 1;
    for(; p > 0; --p) {
        // Megkeressük az első, nem "continuation" bitet; odatesszük a \0-t.
        if ((str[p] & 0xC0) != 0x80) {
            str[p] = '\0';
            break;
        }
    }


    char* edited_str = malloc(len);
    for(size_t i = 0U; i < p; ++i) {
        edited_str[i] = str[i];
    }
    edited_str[p] = '\0';

    free(str);
    str = NULL;

    return edited_str;
}

size_t get_utf8_length(char const* str)
{
    if(str == NULL) return 0U;

    char const* p = str;
    size_t len = 0U;
    while(*p != '\0') {
        if ((*p & 0xC0) != 0x80) {
            ++len;
        }
        ++p;
    }

    return len;
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