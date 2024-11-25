#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "ui.h"
#include "app.h"
#include "utils.h"

#ifdef _CRCLONE_DEBUG
    #include "debug/debugmalloc.h"
#endif

/**
 * @brief Létrehoz egy gombot.
 *
 * Megjegyzés: A lefoglalt memória megfelelő felszabadításához meg kell hívni a `Carcassone__Button__destroy` függvényt.
 *
 * @param this A renderert tartalmazó `Carcassone` struktúra.
 * @param font A címkéhez használt betűtípus.
 * @param label A címke tartalma.
 * @param global_rect A gomb globális pozíciója (az ablakon).
 * @param bg_color A gomb színe.
 * @param text_color A címke színe.
 * @param center A container-ben középre legyen e igazítva a címke.
 * @return Az új `Button`.
 */
Button Carcassone__Button__construct(Carcassone* this, TTF_Font* font, char* label, SDL_Rect global_rect, SDL_Color bg_color, SDL_Color text_color, bool center)
{
    Button new_button = {.label = strdup_(label), .bg_color = bg_color, .global_rect = global_rect, .used_font = font};

    // Címke.
    int rect_width, rect_height;
    TTF_SizeUTF8(font, label, &rect_width, &rect_height);
    new_button.label_rect = (SDL_Rect){
        0, global_rect.h/2 - rect_height/2,
        rect_width, rect_height
    };
    if(center) {
        new_button.label_rect.x = global_rect.w/2 - rect_width/2;
    }

    SDL_Surface* label_surface = TTF_RenderUTF8_Blended(font, label, text_color);
    if(label_surface != NULL) {
        new_button.label_texture = SDL_CreateTextureFromSurface(this->renderer, label_surface);
        SDL_FreeSurface(label_surface);
    }

    return new_button;
}

/**
 * @brief Felszabadítja a megadott `Button` struktúra által lefoglalt memóriát.
 *
 * @param this A `Carcassone` struktúra, aminek az ablakára az adott gomb renderelve van (nem használt, régi verzió miatt).
 * @param button A `Button` struktúra, aminek a lefoglalt memóriáját fel kell szabadítani.
 */
void Carcassone__Button__destroy(Carcassone* this, Button* button)
{
    free(button->label);
    destroy_SDL_Texture(button->label_texture);
}

/**
 * @brief A gomb megjelenítése.
 *
 * @param this A `Carcassone` struktúra, amelyhez a renderer tartozik.
 * @param button A megjelenítendő gomb.
 */
void Carcassone__Button__render(Carcassone* this, Button* button)
{
    SDL_RenderSetViewport(this->renderer, &button->global_rect);
    
    SDL_SetRenderDrawColor(this->renderer, button->bg_color.r, button->bg_color.g, button->bg_color.b, button->bg_color.a);
    SDL_RenderFillRect(this->renderer, NULL);
    if(button->label_texture != NULL) {
        SDL_RenderCopy(this->renderer, button->label_texture, NULL, &button->label_rect);
    }

    SDL_RenderSetViewport(this->renderer, NULL);
}

/**
 * @brief Létrehoz egy szöveginputot.
 *
 * Megjegyzés: A lefoglalt memória megfelelő felszabadításához meg kell hívni a `Carcassone__Prompt__destroy` függvényt.
 *
 * @param this A renderert tartalmazó `Carcassone` struktúra.
 * @param font A címkéhez használt betűtípus.
 * @param default_label A címke tartalma.
 * @param global_rect A gomb globális pozíciója (az ablakon).
 * @param bg_color A gomb színe.
 * @param text_color A címke színe.
 * @return Az új `Prompt`.
 */
Prompt Carcassone__Prompt__construct(Carcassone* this, TTF_Font* font, char* default_label, SDL_Rect global_rect, SDL_Color bg_color, SDL_Color text_color)
{
    Prompt new_prompt = {
        .prompt = Carcassone__Button__construct(this, font, default_label, global_rect, bg_color, text_color, false),
    };

    return new_prompt;
}

/**
 * @brief Felszabadítja a megadott `Prompt` struktúra által lefoglalt memóriát.
 *
 * @param this A `Carcassone` struktúra, aminek az ablakára az adott szöveginput renderelve van (nem használt, régi verzió miatt).
 * @param prompt A `Prompt` struktúra, aminek a lefoglalt memóriáját fel kell szabadítani.
 */
void Carcassone__Prompt__destroy(Carcassone* this, Prompt* prompt)
{
    Carcassone__Button__destroy(this, &prompt->prompt);
}

/**
 * @brief A szöveginput megjelenítése.
 *
 * @param this A `Carcassone` struktúra, amelyhez a renderer tartozik.
 * @param prompt A megjelenítendő szöveginput.
 */
void Carcassone__Prompt__render(Carcassone* this, Prompt* prompt)
{
    Carcassone__Button__render(this, &prompt->prompt);
}

/**
 * @brief Vagy hozzátold a szöveginput címkéjéhez, vagy lecseréli azt.
 *
 * @param this A renderert tartalmazó `Carcassone` struktúra.
 * @param prompt A szöveginput.
 * @param new_label Az új címke vagy a hozzátoldott szöveg.
 * @param concat Hozzátoldás e.
 */
void Carcassone__Prompt__edit(Carcassone* this, Prompt* prompt, char* new_label, bool concat)
{   
    if(concat) {
        if(prompt->prompt.label == NULL || mb_strlen(prompt->prompt.label) == 0) {
            prompt->prompt.label = strdup_(new_label);
        } else {
            prompt->prompt.label = strcatdyn(prompt->prompt.label, new_label);
        }
    } else {
        prompt->prompt.label = mb_remove_last_char_dyn(prompt->prompt.label);
    }

    destroy_SDL_Texture(prompt->prompt.label_texture);

    SDL_Surface* updated_surface = TTF_RenderUTF8_Blended(prompt->prompt.used_font, prompt->prompt.label, (SDL_Color){0, 0, 0, 255});
    if(updated_surface != NULL) {
        prompt->prompt.label_texture = SDL_CreateTextureFromSurface(this->renderer, updated_surface);
        
        int new_width;
        TTF_SizeUTF8(prompt->prompt.used_font, prompt->prompt.label, &new_width, NULL);
        prompt->prompt.label_rect.w = new_width;

        SDL_FreeSurface(updated_surface);
    }
}


