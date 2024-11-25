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

/**
* @brief Multibyte (akár UTF8) karaktereket tartalmazó sztring "látható karaktereinek" számának meghatározása.
*
* @param str A sztring aminek meghatározzuk a hosszát.
* @return A megadott sztring hossza.
*/
size_t mb_strlen(char const* str)
{
    if(str == NULL) return 0;

    size_t mb_len = mbstowcs(NULL, str, 0);
    wchar_t wstr[mb_len + 1];

    mbstowcs(wstr, str, mb_len + 1);

    return wcslen(wstr);
}

/**
* @brief `strdup` POSIX függvényhez hasonló.
*
* @param str A sztring amit dinamikusan le kell foglalni.
* @return Az új, dinamikusan foglalt sztring.
*/
char* strdup_(char const* str)
{
    size_t len = strlen(str) + 1;

    char* dyn_str = malloc(len * sizeof(char));
    if(dyn_str == NULL) return NULL;
    
    strcpy(dyn_str, str);

    return dyn_str;
}

/**
* @brief Dinamikus sztringhez hozzátoldás.
*
* Megjegyzés: Az eredeti sztringet felszabadítja.
*
* @param original A dinamikusan foglalt sztring, amelyhez hozzátoldunk.
* @param to_cat A hozzátoldandó sztring (nem feltétlen dinamikus).
* @return Egy új dinamikusan foglalt sztring, amelynek tartalma az `original` és `to_cat`-é.
*/
char* strcatdyn(char* original, char const* to_cat)
{
    if(original == NULL) return NULL;
    
    size_t original_size = strlen(original);
    size_t cat_size = strlen(to_cat);
    if(original_size == 0 || cat_size == 0) return NULL;
    size_t len = original_size + cat_size + 1;

    char* new_str = malloc(len * sizeof(char));
    if(new_str == NULL) return NULL;
    
    strncpy(new_str, original, original_size);
    strncpy(new_str + original_size, to_cat, cat_size + 1);
    free(original);
    original = NULL;


    return new_str;
}

/**
* @brief Dinamikus sztring utolsó karakterének (multibyte) eltávolítása.
*
* Megjegyzés: Az eredeti sztringet felszabadítja.
*
* @param str A dinamikusan foglalt sztring.
* @return Egy új dinamikusan foglalt sztring, amelynek tartalma az `str`-ével megegyezik, kivéve annak utolsó karaktere.
*/
char* mb_remove_last_char_dyn(char* str)
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

    char* edited_str = malloc(len * sizeof(char));
    for(size_t i = 0U; i < p; ++i) {
        edited_str[i] = str[i];
    }
    edited_str[p] = '\0';

    free(str);
    str = NULL;

    return edited_str;
}

/**
* @brief `SDL_Texture` felszabadítása, de kezeli azt is ha NULL a megadott textúra.
*
* @param texture A törlendő textúra.
*/
void destroy_SDL_Texture(SDL_Texture* texture)
{
    if(texture == NULL) return;

    SDL_DestroyTexture(texture);
    texture = NULL;
}

/**
* @brief `SDL_Texture` létrehozása egy BMP képfájlból.
*
* Megjegyzés: A létrejött textúra a `destroy_SDL_Texture`-rel törlendő.
*
* @param renderer A főablakhoz tartozó renderer.
* @param source_path A BMP képfájl elérési útvonala.
* @return A létrehozott textúrára mutató pointer.
*/

SDL_Texture* create_SDL_texture_from_BMP(SDL_Renderer* renderer, char const* source_path)
{
    SDL_Surface* src_surface = SDL_LoadBMP(source_path);
    if(src_surface == NULL) return NULL;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, src_surface);
    SDL_FreeSurface(src_surface);

    return texture;
}