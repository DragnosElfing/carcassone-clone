#include <SDL2/SDL_render.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "app.h"
#include "game/player.h"
#include "game/meeple.h"

#ifdef _CRCLONE_DEBUG
    #include "debug/debugmalloc.h"
#endif

Meeple Meeple__construct(SDL_Renderer* renderer)
{
    Meeple new_meeple = {
        .x = -1,
        .y = -1,
        .is_placed = false
    };

    SDL_Surface* texture_img = SDL_LoadBMP("./res/meeple_base.bmp");
    if(texture_img != NULL) {
        new_meeple.texture = SDL_CreateTextureFromSurface(renderer, texture_img);
        SDL_FreeSurface(texture_img);
    }

    return new_meeple;
}
void Meeple__reclaim(Meeple* this)
{
    this->is_placed = false;
}

void Meeple__destroy(Meeple* this)
{
    if(this->texture != NULL) SDL_DestroyTexture(this->texture);
}

Player Player__construct(SDL_Renderer* renderer, TTF_Font* font, char* name)
{   
    // Alapból tudott értékek
    Player new_player = {
        .score = 0U,
        .has_placed_card = false,
        .meeples_at_hand = MAX_MEEPLES,
        .update_score = true
    };
    strcpy(new_player.name, get_utf8_length(name) <= 24 ? name : "INVALID");

    // Meeple-k létrehozása a játékos számára
    for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
        new_player.meeples[m] = Meeple__construct(renderer);
    }

    // Stat panel létrehozása
    // Név handle létrehozása
    SDL_Surface* handle_surface = TTF_RenderUTF8_Blended_Wrapped(font, new_player.name, (SDL_Color){0, 0, 0, 255}, 300);
    if(handle_surface != NULL) {
        new_player.handle_texture = SDL_CreateTextureFromSurface(renderer, handle_surface);
        SDL_FreeSurface(handle_surface);
    }

    // Pontszámoló létrehozása
    char score_string[2] = "0";
    sprintf(score_string, "%u", new_player.score);
    
    SDL_Surface* score_surface = TTF_RenderUTF8_Blended(font, score_string, (SDL_Color){0, 0, 0, 255});
    new_player.score_counter = NULL;
    if(score_surface != NULL) {
        new_player.score_counter = SDL_CreateTextureFromSurface(renderer, score_surface);
        SDL_FreeSurface(score_surface);
    }

    // Magának a panelnek a létrehozása
    new_player.stat_panel = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 300, 700);

    return new_player;
}

void Player__render(Player* this, SDL_Renderer* renderer, TTF_Font* font)
{
    if(this->stat_panel != NULL) {
        SDL_SetRenderTarget(renderer, this->stat_panel);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        SDL_SetRenderDrawColor(renderer, COLOR_WHITE);
        SDL_RenderFillRect(renderer, NULL);

        char score_str[5+1];
        snprintf(score_str, 6, "%u", this->score);
        if(this->update_score) {
            SDL_Surface* score_surface = TTF_RenderUTF8_Blended(font, score_str, (SDL_Color){0, 0, 0, 255});
            if(score_surface != NULL) {
                SDL_DestroyTexture(this->score_counter);
                this->score_counter = SDL_CreateTextureFromSurface(renderer, score_surface);
                SDL_FreeSurface(score_surface);
            }

            this->update_score = false;
        }

        int w, h;
        TTF_SizeUTF8(font, this->name, &w, &h);
        if(this->handle_texture != NULL) 
            SDL_RenderCopy(renderer, this->handle_texture, NULL, &(SDL_Rect){0, 0, MIN(300, w), h});
        TTF_SizeUTF8(font, score_str, &w, &h);
        if(this->score_counter != NULL) {
            SDL_RenderCopy(renderer, this->score_counter, NULL, 
                &(SDL_Rect){15, 95, w, h});
        }

        unsigned int offset = 0U;
        for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
            if(this->meeples[m].is_placed) continue;

            SDL_RenderCopy(renderer, this->meeples[m].texture, NULL, 
                &(SDL_Rect){10 + 30*offset, 200, 64, 64});
            ++offset;
        }

        SDL_SetRenderTarget(renderer, NULL);
    }
}

void Player__place_meeple(Player* this, SDL_Point tile_index)
{
    if(this->meeples_at_hand <= 0) return;
    // ! Should not check if tile is valid or not

    Meeple* to_be_placed = NULL;
    for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
        if(!this->meeples[m].is_placed) {
            to_be_placed = &this->meeples[m];
            break;
        }
    }

    if(to_be_placed != NULL) {
        to_be_placed->is_placed = true;
        to_be_placed->x = tile_index.x;
        to_be_placed->y = tile_index.y;
        --this->meeples_at_hand;
    }
}

void Player__reclaim_meeple(Player* this, Meeple* to_reclaim)
{
    if(to_reclaim == NULL || this->meeples_at_hand >= MAX_MEEPLES) return;

    ++this->meeples_at_hand;
    to_reclaim->is_placed = false;
}

void Player__toggle_turn_active(Player* this)
{
    //
}

void Player__add_to_score(Player* this, SDL_Renderer* renderer, TTF_Font* font, unsigned int add)
{
    this->score += add;
    this->update_score = true;
    //Player__render(this, renderer, font);
}

void Player__destroy(Player* this)
{
    for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
        Meeple__destroy(&this->meeples[m]);
    }
    if(this->stat_panel != NULL) SDL_DestroyTexture(this->stat_panel);
    if(this->handle_texture != NULL) SDL_DestroyTexture(this->handle_texture); 
    if(this->score_counter != NULL) SDL_DestroyTexture(this->score_counter);
}