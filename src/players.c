#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>

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
        SDL_SetTextureBlendMode(new_meeple.texture, SDL_BLENDMODE_BLEND);
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

Player Player__construct(SDL_Renderer* renderer, TTF_Font* font, char* name, unsigned int highscore)
{   
    // Alapból tudott értékek
    Player new_player = {
        .highscore = highscore,
        .score = 0U,
        .has_placed_card = false,
        .has_placed_meeple = false,
        .is_turn_active = false,
        .meeples_at_hand = MAX_MEEPLES
    };
    strcpy(new_player.name, get_utf8_length(name) <= 24 ? name : "INVALID");

    // Meeple-k létrehozása a játékos számára
    for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
        new_player.meeples[m] = Meeple__construct(renderer);
    }

    // Stat panel létrehozása
    // Név handle létrehozása
    SDL_Surface* handle_surface = TTF_RenderUTF8_Blended_Wrapped(font, new_player.name, (SDL_Color){0, 0, 0, 255}, 300);
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

        int w, h;
        TTF_SizeUTF8(font, new_player.name, &w, &h);
        if(handle != NULL) SDL_RenderCopy(renderer, handle, NULL, &(SDL_Rect){0, 0, MIN(300, w), h});
        TTF_SizeUTF8(font, score_string, &w, &h);
        if(new_player.score_counter != NULL) {
            SDL_RenderCopy(renderer, new_player.score_counter, NULL, 
                &(SDL_Rect){15, 95, w, h});
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

void Player__place_meeple(Player* this, SDL_Point tile_index)
{
    if(this->meeples_at_hand <= 0) return;
    // ! Should not check if tile is valid or not

    Meeple* to_be_placed = &this->meeples[--this->meeples_at_hand];

    to_be_placed->is_placed = true;
    to_be_placed->x = tile_index.x;
    to_be_placed->y = tile_index.y;

    DBG_LOG("%zu", this->meeples_at_hand);
}

void Player__reclaim_meeple(Player* this, Meeple* to_reclaim)
{
    if(to_reclaim == NULL || this->meeples_at_hand >= MAX_MEEPLES) return;

    ++this->meeples_at_hand;
    to_reclaim->is_placed = false;

    DBG_LOG("%zu", this->meeples_at_hand);
}

void Player__toggle_turn_active(Player* this)
{
    this->is_turn_active = !this->is_turn_active;
}

void Player__add_to_score(Player* this, unsigned int add)
{
    this->score += add;
    // TODO
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