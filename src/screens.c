#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "game/player.h"
#include "game/tile.h"
#include "ui.h"
#include "app.h"

void Carcassone__Menu__construct(Carcassone* this)
{
    this->menu_screen = malloc(sizeof(MenuScreen));
    SDL_Surface* menu_background_image = SDL_LoadBMP("./res/menu_bg.bmp");
    if(menu_background_image != NULL) {
        this->menu_screen->background = SDL_CreateTextureFromSurface(this->renderer, menu_background_image);
        if(this->menu_screen->background == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Nem lehetett betölteni az háttérképet!");
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "%s", SDL_GetError());
        }
        SDL_FreeSurface(menu_background_image);
    }
    SDL_SetRenderDrawBlendMode(this->renderer, SDL_BLENDMODE_BLEND);

    // TODO: rel coords, viewport
    this->menu_screen->button_container = (SDL_FRect){this->width/2.0f - 400, 250, 800, 600};

    // Create buttons
    SDL_Rect start_button_rect = {
        this->menu_screen->button_container.x + this->menu_screen->button_container.w / 2 - 100,
        this->menu_screen->button_container.y + 100,
        200, 60
    };
    this->menu_screen->start_button = 
        Carcassone__Button__construct(this, "START", start_button_rect, (SDL_Color){COLOR_BLUE}, (SDL_Color){COLOR_WHITE});

    SDL_Rect lboard_button_rect = {
        this->menu_screen->button_container.x + this->menu_screen->button_container.w / 2 - 200,
        this->menu_screen->button_container.y + 200,
        400, 60
    };
    this->menu_screen->lboard_button = 
        Carcassone__Button__construct(this, "DICSŐSÉGLISTA", lboard_button_rect, (SDL_Color){COLOR_BLUE}, (SDL_Color){COLOR_WHITE});
}
void Carcassone__Menu__destroy(Carcassone* this)
{
    if(this->menu_screen->background != NULL) SDL_DestroyTexture(this->menu_screen->background);
    Carcassone__Button__destroy(this, &this->menu_screen->start_button);
    Carcassone__Button__destroy(this, &this->menu_screen->lboard_button);
    free(this->menu_screen);
}
void Carcassone__Lboard__construct(Carcassone* this)
{
    this->lboard_screen = malloc(sizeof(LeaderboardScreen));
    // Load and sort leaderboard data
    this->lboard_screen->leaderboard = Leaderboard__construct("res/data/records.dat");

    this->lboard_screen->back_button = 
        Carcassone__Button__construct(this, "VISSZA", (SDL_Rect){this->width - 150, 10, 140, 50}, (SDL_Color){COLOR_WHITE}, 
            (SDL_Color){0, 0, 0, 255});

    // Texture for the leaderboard entries
    this->lboard_screen->list_texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
        1000, this->height - 200);
    SDL_SetRenderTarget(this->renderer, this->lboard_screen->list_texture);

    if(this->lboard_screen->leaderboard != NULL) {
        Leaderboard__sort(this->lboard_screen->leaderboard);
        char score_string[10+1];
        int w, h;
        for(size_t place = 0U; place < MIN(5, this->lboard_screen->leaderboard->entries_size); ++place) {
            char* curr_name = this->lboard_screen->leaderboard->entries[place].name;
            TTF_SizeUTF8(this->default_font, curr_name, &w, &h);
            SDL_RenderCopy(this->renderer, SDL_CreateTextureFromSurface(
                this->renderer, TTF_RenderUTF8_Blended(this->default_font, this->lboard_screen->leaderboard->entries[place].name,
                (SDL_Color){COLOR_WHITE})), 
                NULL, &(SDL_Rect){0, 100*place, 
                    w * 0.4, 
                    h * 0.4});

            sprintf(score_string, "%u", this->lboard_screen->leaderboard->entries[place].highscore);
            SDL_RenderCopy(this->renderer, SDL_CreateTextureFromSurface(
                this->renderer, TTF_RenderUTF8_Blended(this->default_font, score_string,
                (SDL_Color){COLOR_WHITE})), NULL, &(SDL_Rect){900, 100*place, 
                    50 / (this->lboard_screen->leaderboard->entries[place].highscore < 10 ? 2 : 1), 80});
        }
    } else {
        int w, h;
        TTF_SizeUTF8(this->default_font, "Hibás fájlformátum!", &w, &h);
        SDL_RenderCopy(this->renderer, SDL_CreateTextureFromSurface(
            this->renderer, TTF_RenderUTF8_Blended(this->default_font, 
            "Hibás fájlformátum!", (SDL_Color){COLOR_WHITE})), NULL, 
            &(SDL_Rect){100, 100, w*0.6f, h*0.6f});
    }

    SDL_SetRenderTarget(this->renderer, NULL);
    SDL_SetTextureBlendMode(this->lboard_screen->list_texture, SDL_BLENDMODE_BLEND);
}
void Carcassone__Lboard__destroy(Carcassone* this)
{
    Leaderboard__destroy(this->lboard_screen->leaderboard);
    if(this->lboard_screen->list_texture != NULL) SDL_DestroyTexture(this->lboard_screen->list_texture);
    Carcassone__Button__destroy(this, &this->lboard_screen->back_button);
    free(this->lboard_screen);
}

void Carcassone__Menu__render(Carcassone* this)
{
    SDL_RenderClear(this->renderer);
    SDL_RenderCopy(this->renderer, this->menu_screen->background, NULL, NULL);

    SDL_SetRenderDrawColor(this->renderer, 255, 165, 105, 155);
    SDL_RenderFillRectF(this->renderer, NULL);

    Carcassone__Button__render(this, &this->menu_screen->start_button);
    Carcassone__Button__render(this, &this->menu_screen->lboard_button);

    SDL_RenderCopy(this->renderer, this->splash_title, NULL,
        &(SDL_Rect){this->width/2.0f - 400, 0, 800, 240});

    SDL_RenderPresent(this->renderer);
}
void Carcassone__Lboard__render(Carcassone* this)
{
    SDL_RenderClear(this->renderer);

    Carcassone__Button__render(this, &this->lboard_screen->back_button);

    SDL_RenderCopy(this->renderer, this->splash_title, NULL,
        &(SDL_Rect){this->width/2.0f - 400, 0, 800, 240});

    SDL_RenderCopy(this->renderer, this->lboard_screen->list_texture, NULL,
        &(SDL_Rect){this->width/2.0f - 500, 300, 1000, 700});

    SDL_SetRenderDrawColor(this->renderer, COLOR_BLUE);
    SDL_RenderPresent(this->renderer);
}

void Carcassone__Game__construct(Carcassone* this)
{
    this->game_screen = malloc(sizeof(GameScreen));
    this->game_screen->map_offset = (SDL_Point){this->width/2-300, 120};
    this->game_screen->board = NULL;
    this->game_screen->pile_index = 0U;
    this->game_screen->is_ready = false;

    char counter_string[2] = "0";
    SDL_Surface* curr_index_surface = TTF_RenderText_Blended(this->default_font, counter_string, (SDL_Color){COLOR_WHITE});
    for(size_t ti = 0U; ti < PILE_SIZE; ++ti) {
        sprintf(counter_string, "%zu", PILE_SIZE - ti);
        curr_index_surface = TTF_RenderText_Blended(this->default_font, counter_string, (SDL_Color){COLOR_WHITE});
        if(curr_index_surface != NULL) {
            this->game_screen->pile_counter[ti] = SDL_CreateTextureFromSurface(this->renderer, curr_index_surface);
        }

        if(this->game_screen->pile_counter[ti] == NULL)
            break;
    }
    SDL_FreeSurface(curr_index_surface);

    this->game_screen->board_texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET, 600, this->height - this->game_screen->map_offset.y - 10);
    this->game_screen->tileset_wrapper = TilesetWrapper__construct(this->renderer);
    if(this->game_screen->tileset_wrapper.tile_set == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Nem lehetett betölteni az atlaszt!");
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "%s", SDL_GetError());
    }
    Carcassone__init_board(this);
    Carcassone__init_pile(this);
    Carcassone__draw_new(this);

    SDL_Rect input1_rect = {
        this->width / 4 - 150, 200, 300, 100
    };
    SDL_Rect input2_rect = {
        3 * this->width / 4 - 150, 200, 300, 100
    };
    this->game_screen->player_name_inputs[0] = 
        Carcassone__Prompt__construct(this, "Játékos1", true, input1_rect, (SDL_Color){COLOR_WHITE}, (SDL_Color){0, 0, 0, 255});
    this->game_screen->player_name_inputs[1] = 
        Carcassone__Prompt__construct(this, "Játékos2", false, input2_rect, (SDL_Color){COLOR_WHITE}, (SDL_Color){0, 0, 0, 255});
    this->game_screen->ready_button = 
        Carcassone__Button__construct(this, "START", (SDL_Rect){10, this->height - 10 - 60, 150, 60}, (SDL_Color){COLOR_WHITE}, 
            (SDL_Color){0, 0, 0, 255});
}

void Carcassone__Game__destroy(Carcassone* this)
{
    if(this->game_screen->board_texture != NULL) SDL_DestroyTexture(this->game_screen->board_texture);
    for(size_t n = 0U; n < PILE_SIZE; ++n) {
        if(this->game_screen->pile_counter[n] != NULL) SDL_DestroyTexture(this->game_screen->pile_counter[n]);
    }

    if(this->game_screen->board != NULL) {
        for(size_t n = 0U; n < BOARD_SIZE; ++n) {
            free(this->game_screen->board[n]);
        }
        free(this->game_screen->board);
    }
    for(size_t n = 0U; n < PILE_SIZE; ++n) {
        free(this->game_screen->card_pile[n]);
    }
    TilesetWrapper__destroy(&this->game_screen->tileset_wrapper);
    free(this->game_screen);
}

void Carcassone__Game__render(Carcassone* this)
{
    SDL_SetRenderDrawColor(this->renderer, 102, 102, 153, 255);
    SDL_RenderClear(this->renderer);

    if(!this->game_screen->is_ready) {
        Carcassone__Prompt__render(this, &this->game_screen->player_name_inputs[0]);
        Carcassone__Prompt__render(this, &this->game_screen->player_name_inputs[1]);
        Carcassone__Button__render(this, &this->game_screen->ready_button);
    } else {
        Carcassone__render_board(this);
        Carcassone__render_splash_title(this);
        Carcassone__render_player_stats(this);
        Carcassone__render_drawn_tile(this);
    }

    SDL_RenderPresent(this->renderer);
}