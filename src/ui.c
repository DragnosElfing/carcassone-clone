#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "ui.h"
#include "app.h"

Button Carcassone__Button__construct(Carcassone* this, char* label, SDL_Rect global_rect, SDL_Color bg_color, SDL_Color text_color)
{
    Button new_button = {.label = label, .bg_color = bg_color, .global_rect = global_rect};

    int rect_width, rect_height;
    TTF_SizeUTF8(this->default_font, label, &rect_width, &rect_height);
    new_button.rect = (SDL_Rect){
        0, 0,
        rect_width, rect_height
    };

    SDL_Surface* label_surface = TTF_RenderUTF8_Blended(this->default_font, label, text_color);
    if(label_surface != NULL) {
        new_button.label_texture = SDL_CreateTextureFromSurface(this->renderer, label_surface);
        SDL_FreeSurface(label_surface);
    }

    return new_button;
}

void Carcassone__Button__render(Carcassone* this, Button* button)
{
    SDL_RenderSetViewport(this->renderer, &button->global_rect);
    SDL_SetRenderDrawColor(this->renderer, button->bg_color.r, button->bg_color.g, button->bg_color.b, button->bg_color.a);
    SDL_RenderFillRect(this->renderer, &button->rect);
    SDL_RenderCopy(this->renderer,button->label_texture, NULL, NULL);
    SDL_RenderSetViewport(this->renderer, NULL);
}

void Carcassone__Button__destroy(Carcassone* this, Button* button)
{
    if(button->label_texture != NULL) SDL_DestroyTexture(button->label_texture);
}

Prompt Carcassone__Prompt__construct(Carcassone* this, char* default_label, bool autofocus, SDL_Rect global_rect, SDL_Color bg_color, SDL_Color text_color)
{
    char* label = malloc(24 * sizeof(char));
    for(size_t i = 0U; i <= strlen(default_label); ++i) {
        label[i] = default_label[i];
    }
    Prompt new_prompt = {
        .prompt = Carcassone__Button__construct(this, label, global_rect, bg_color, text_color),
        .is_active = autofocus
    };

    return new_prompt;
}

void Carcassone__Prompt__edit(Carcassone* this, Prompt* prompt, char* new_label)
{
    strcat(prompt->prompt.label, new_label);

    SDL_Surface* updated_surface = TTF_RenderUTF8_Blended(this->default_font, prompt->prompt.label, (SDL_Color){0, 0, 0, 255});
    if(updated_surface != NULL) {
        prompt->prompt.label_texture = SDL_CreateTextureFromSurface(this->renderer, updated_surface);
        SDL_FreeSurface(updated_surface);
    }
}

void Carcassone__Prompt__toggle_focus(Carcassone* this, Prompt* prompt)
{
    prompt->is_active = !prompt->is_active;
}

void Carcassone__Prompt__render(Carcassone* this, Prompt* prompt)
{
    Carcassone__Button__render(this, &prompt->prompt);
}

void Carcassone__Prompt__destroy(Carcassone* this, Prompt* prompt)
{
    free(prompt->prompt.label);
    Carcassone__Button__destroy(this, &prompt->prompt);
}
