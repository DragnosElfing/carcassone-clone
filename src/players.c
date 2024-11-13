#include "game/player.h"
#include "game/meeple.h"
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>

Meeple Meeple__construct(SDL_Renderer* renderer)
{
    Meeple new_meeple = {
        .x = -1,
        .y = -1,
        .tile = NULL,
        .is_placed = false
    };

    SDL_Surface* texture_img = SDL_LoadBMP("./res/meeple_base.bmp");
    if(texture_img != NULL) {
        new_meeple.texture = SDL_CreateTextureFromSurface(renderer, texture_img);
        SDL_SetTextureBlendMode(new_meeple.texture, SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(texture_img);
    }

    return new_meeple;
}
void Meeple__place_on(Meeple*, unsigned int, unsigned int);
void Meeple__reclaim(Meeple*);

void Meeple__destroy(Meeple* this)
{
    if(this->texture != NULL) SDL_DestroyTexture(this->texture);
}

Player Player__construct(SDL_Renderer* renderer, TTF_Font* font, char const* name, unsigned int highscore)
{   
    // Alapból tudott értékek
    Player new_player = {
        .highscore = highscore,
        .score = 0U,
        .has_placed_card = false,
        .is_turn_active = false,
        .meeples_at_hand = MAX_MEEPLES
    };
    strcpy(new_player.name, strlen(name) <= 24 ? name : "INVALID");

    // Meeple-k létrehozása a játékos számára
    for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
        new_player.meeples[m] = Meeple__construct(renderer);
    }

    // Stat panel létrehozása
    // Név handle létrehozása
    SDL_Surface* handle_surface = TTF_RenderUTF8_Blended(font, new_player.name, (SDL_Color){0, 0, 0, 255});
    SDL_Texture* handle = NULL;
    if(handle_surface != NULL) {
        handle = SDL_CreateTextureFromSurface(renderer, handle_surface);
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

    // Magának a panelnek a létrehozása, és rárenderelés
    new_player.stat_panel = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 300, 700);
    if(new_player.stat_panel != NULL) {
        SDL_SetRenderTarget(renderer, new_player.stat_panel);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, NULL);

        if(handle != NULL) SDL_RenderCopy(renderer, handle, NULL, &(SDL_Rect){0, 0, 300, 60});
        if(new_player.score_counter != NULL) {
            SDL_RenderCopy(renderer, new_player.score_counter, NULL, 
                &(SDL_Rect){15, 95, 50, 80});
        }
        for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
            SDL_RenderCopy(renderer, new_player.meeples[m].texture, NULL, 
                &(SDL_Rect){10 + 30*m, 200, 64, 64});
        }

        SDL_SetRenderTarget(renderer, NULL);
    }
    if(handle != NULL) SDL_DestroyTexture(handle);

    return new_player;
}

void Player__toggle_turn_active(Player* this)
{
    this->is_turn_active = !this->is_turn_active;
}

void Player__update_score(Player* this, unsigned int new_score)
{
    this->score = new_score;
    // Pontszámoló textúra frissítése
    // SDL_Surface* score_surface = TTF_RenderUTF8_Blended(font, this->score, (SDL_Color){0, 0, 0, 255});
    // if(score_surface != NULL) {
    //     this->score_counter = SDL_CreateTextureFromSurface(renderer, handle_surface);
    //     SDL_FreeSurface(score_surface);
    // }

}

void Player__destroy(Player* this)
{
    for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
        Meeple__destroy(&this->meeples[m]);
    }
    if(this->stat_panel != NULL) SDL_DestroyTexture(this->stat_panel);
    if(this->score_counter != NULL) SDL_DestroyTexture(this->score_counter);
    free(this);
}