#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "utils.h"
#include "app.h"
#include "game/player.h"
#include "game/meeple.h"

#ifdef _CRCLONE_DEBUG
    #include "debug/debugmalloc.h"
#endif

/**
 * @brief Létrehoz egy alattvalót.
 *
 * Megjegyzés: A lefoglalt memória megfelelő felszabadításához meg kell hívni a `Meeple__destroy` függvényt.
 *
 * @param texture A kinézete.
 * @return Az új `Meeple` struktúra.
 */
Meeple Meeple__construct(SDL_Texture* texture)
{
    Meeple new_meeple = {
        .x = -1,
        .y = -1,
        .is_placed = false,
        .texture = texture
    };

    return new_meeple;
}

// /**
//  * @brief Felszabadítja a megadott `Meeple` struktúra által lefoglalt memóriát.
//  *
//  * @param this A `Meeple` struktúra, aminek a lefoglalt memóriáját fel kell szabadítani.
//  */
// void Meeple__destroy(Meeple* this)
// {
//     destroy_SDL_Texture(this->texture);
// }

Player Player__construct(SDL_Renderer* renderer, TTF_Font* font, char* name, char const* meeple_outfit)
{   
    Player new_player = {
        .score = 0U,
        .has_placed_card = false,
        .meeples_at_hand = MAX_MEEPLES,
        .update_score = true
    };
    strcpy(new_player.name, get_utf8_length(name) <= 24 ? name : "INVALID");

    // Alattvalók létrehozása a játékos számára.
    new_player.own_meeple_texture = create_SDL_texture_from_BMP(renderer, meeple_outfit);
    if(new_player.own_meeple_texture != NULL) {
        for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
            new_player.meeples[m] = Meeple__construct(new_player.own_meeple_texture);
        }
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

/**
 * @brief Felszabadítja a megadott `Player` struktúra által lefoglalt memóriát.
 *
 * @param this A `Player` struktúra, aminek a lefoglalt memóriáját fel kell szabadítani.
 */
void Player__destroy(Player* this)
{
    destroy_SDL_Texture(this->stat_panel); // TODO: ENNÉL VAN A HIBA
    destroy_SDL_Texture(this->handle_texture);
    destroy_SDL_Texture(this->score_counter);
    destroy_SDL_Texture(this->own_meeple_texture);
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

/**
 * @brief Alattvaló lehelyezése.
 *
 * Nem viszgálja meg, hogy a mezőkártya valóban megfelelő e.
 *
 * @param this Az a `Player`, amelynek egy alattvalóját le kell helyezni.
 * @param tile_index A mezőkártya koordinátája a játéktáblán, amire az alattvaló le lesz téve.
 */
void Player__place_meeple(Player* this, SDL_Point tile_index)
{
    if(this->meeples_at_hand <= 0) return;

    // Az első nem-lehelyezett alattvaló kiválasztása.
    Meeple* to_be_placed = NULL;
    for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
        if(!this->meeples[m].is_placed) {
            to_be_placed = &this->meeples[m];
            break;
        }
    }

    // Lehelyezés
    if(to_be_placed != NULL) {
        to_be_placed->is_placed = true;
        to_be_placed->x = tile_index.x;
        to_be_placed->y = tile_index.y;
        --this->meeples_at_hand;
    }
}

/**
 * @brief Lehelyezett alattvaló visszaszerzése.
 *
 * @param this A `Player` struktúra, amelynek egy alattvalóját vissza kell szerezni.
 * @param to_reclaim A visszaszerzendő `Meeple`.
 */
void Player__reclaim_meeple(Player* this, Meeple* to_reclaim)
{
    if(to_reclaim == NULL || !to_reclaim->is_placed || this->meeples_at_hand >= MAX_MEEPLES) return;

    ++this->meeples_at_hand;
    to_reclaim->is_placed = false;
}

/**
 * @brief Játékos pontszámának növelése.
 *
 * Beállítja az `update_score`-t, a következő render esetén frissíti a számlálót.
 *
 * @param this A `Player`, amelynek a pontszámát növeljük.
 * @param add A hozzáadott mennyiség.
 */
void Player__add_to_score(Player* this, unsigned int add)
{
    this->score += add;
    this->update_score = true;
}