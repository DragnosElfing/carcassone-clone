#include "game/player.h"
#include "game/meeple.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
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

Player* Player__construct(SDL_Renderer* renderer, char* name, unsigned int highscore)
{
    Player* new_player = malloc(sizeof(Player));

    strcpy(new_player->name, strlen(name) <= 24 ? name : "INVALID");
    new_player->highscore = highscore;
    new_player->score = 0U;
    for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
        new_player->meeples[m] = Meeple__construct(renderer);
    }

    return new_player;
}

void Player__toggle_turn_active(Player*);
void Player__update_score(Player*, unsigned int);

void Player__destroy(Player* this)
{
    for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
        Meeple__destroy(&this->meeples[m]);
    }
    free(this);
}