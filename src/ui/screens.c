#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "utils.h"
#include "game/player.h"
#include "game/tile.h"
#include "ui.h"
#include "app.h"

#ifdef _CRCLONE_DEBUG
    #include "debug/debugmalloc.h"
#endif

/**
 * @brief Létrehozza a menünézetet.
 *
 * Megjegyzés: A lefoglalt memória megfelelő felszabadításához meg kell hívni a `Carcassone__Menu__destroy` függvényt.
 *
 * @param this A `Carcassone` struktúra, amelynek létrehozza a menünézetét.
 */
void Carcassone__Menu__construct(Carcassone* this)
{
    this->menu_screen = malloc(sizeof(MenuScreen));
    this->menu_screen->background = create_SDL_texture_from_BMP(this->renderer, "./res/menu_bg.bmp");

    // A háttérkép transzparenciája miatt.
    SDL_SetRenderDrawBlendMode(this->renderer, SDL_BLENDMODE_BLEND);

    this->menu_screen->button_container = (SDL_Rect){this->width/2 - 400, 250, 800, 600};

    // Gombok létrehozása
    SDL_Rect start_button_rect = {
        this->menu_screen->button_container.x + this->menu_screen->button_container.w / 2 - 100,
        this->menu_screen->button_container.y + 100,
        200, 60
    };
    this->menu_screen->start_button = 
        Carcassone__Button__construct(this, this->default_font, "START", start_button_rect, (SDL_Color){COLOR_BLUE}, (SDL_Color){COLOR_WHITE}, true);

    SDL_Rect lboard_button_rect = {
        this->menu_screen->button_container.x + this->menu_screen->button_container.w / 2 - 200,
        this->menu_screen->button_container.y + 200,
        400, 60
    };
    this->menu_screen->lboard_button = 
        Carcassone__Button__construct(this, this->default_font, "DICSŐSÉGLISTA", lboard_button_rect, (SDL_Color){COLOR_BLUE}, (SDL_Color){COLOR_WHITE}, true);
}

/**
 * @brief Felszabadítja a megadott `Carcassone` struktúrához tartozó `MenuScreen` által lefoglalt memóriát.
 *
 * @param this A `Carcassone` struktúra, aminek a lefoglalt memóriáját fel kell szabadítani.
 */
void Carcassone__Menu__destroy(Carcassone* this)
{
    destroy_SDL_Texture(this->menu_screen->background);
    Carcassone__Button__destroy(this, &this->menu_screen->start_button);
    Carcassone__Button__destroy(this, &this->menu_screen->lboard_button);
    free(this->menu_screen);
}

/**
 * @brief Menünézet megjelenítése.
 *
 * @param this A `Carcassone` struktúra, amihez a menü tartozik.
 */
void Carcassone__Menu__render(Carcassone* this)
{
    SDL_RenderClear(this->renderer);

    // Háttér.
    SDL_RenderCopy(this->renderer, this->menu_screen->background, NULL, NULL);
    SDL_SetRenderDrawColor(this->renderer, 235, 235, 225, 100);
    SDL_RenderFillRect(this->renderer, NULL);

    Carcassone__Button__render(this, &this->menu_screen->start_button);
    Carcassone__Button__render(this, &this->menu_screen->lboard_button);

    // TODO: a splash_title minden nézetben ugyanott van
    SDL_RenderCopy(this->renderer, this->splash_title, NULL,
        &(SDL_Rect){this->width/2 - 400, 0, 800, 240});
}

/**
 * @brief Létrehozza a dicsőséglistanézetet.
 *
 * Megjegyzés: A lefoglalt memória megfelelő felszabadításához meg kell hívni a `Carcassone__Lboard__destroy` függvényt.
 *
 * @param this A `Carcassone` struktúra, amelynek létrehozza a dicsőséglistanézetét.
 */
void Carcassone__Lboard__construct(Carcassone* this)
{
    this->lboard_screen = malloc(sizeof(LeaderboardScreen));
    this->lboard_screen->leaderboard = Leaderboard__construct("res/data/records.dat");
    this->lboard_screen->list_texture = NULL;
    strcpy(this->lboard_screen->syntax_error_msg, "Hibás fájlformátum!");

    // "Vissza" gomb.
    this->lboard_screen->back_button = 
        Carcassone__Button__construct(this, this->small_font, "VISSZA", (SDL_Rect){this->width - 150, 10, 140, 50}, 
        (SDL_Color){COLOR_WHITE}, (SDL_Color){0, 0, 0, 255}, true);

    Carcassone__Lboard__init_list_texture(this);
}

/**
 * @brief Felszabadítja a megadott `Carcassone` struktúrához tartozó `LeaderboardScreen` által lefoglalt memóriát.
 *
 * @param this A `Carcassone` struktúra, aminek a lefoglalt memóriáját fel kell szabadítani.
 */
void Carcassone__Lboard__destroy(Carcassone* this)
{
    Leaderboard__destroy(this->lboard_screen->leaderboard);
    destroy_SDL_Texture(this->lboard_screen->list_texture);
    Carcassone__Button__destroy(this, &this->lboard_screen->back_button);
    free(this->lboard_screen);
}

/**
 * @brief Létrehozza a dicsőséglistanézethez a rekordokat.
 *
 * Akkor kell meghívni, ha frissül a rekordfájl.
 *
 * @param this A `Carcassone` struktúra, amelynek létrehozza a dicsőséglistanézetét.
 */
void Carcassone__Lboard__init_list_texture(Carcassone* this)
{
    // A rekordok.
    destroy_SDL_Texture(this->lboard_screen->list_texture);
    this->lboard_screen->list_texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
        1000, this->height - 200);
    SDL_SetRenderTarget(this->renderer, this->lboard_screen->list_texture);

    if(this->lboard_screen->leaderboard != NULL) {
        Leaderboard__sort(this->lboard_screen->leaderboard);
        
        char score_string[10+1]; // safe bet
        int w, h;
        // A top 5 rekord megjelenítése.
        for(size_t place = 0U; place < MIN(5, this->lboard_screen->leaderboard->entries_size); ++place) {
            // A rekordhoz tartozó játékos neve.
            char* curr_name = this->lboard_screen->leaderboard->entries[place].name;
            TTF_SizeUTF8(this->small_font, curr_name, &w, &h);
            SDL_Surface* name_surface = TTF_RenderUTF8_Blended(this->small_font, this->lboard_screen->leaderboard->entries[place].name,
                (SDL_Color){COLOR_WHITE});
            if(name_surface != NULL) {
                SDL_Texture* name = SDL_CreateTextureFromSurface(this->renderer, name_surface);
                SDL_RenderCopy(this->renderer, name, 
                    NULL, &(SDL_Rect){0, 160 * place, w, h});
                SDL_FreeSurface(name_surface);
                destroy_SDL_Texture(name);
            }
            
            // A rekord.
            sprintf(score_string, "%u", this->lboard_screen->leaderboard->entries[place].highscore);
            TTF_SizeUTF8(this->small_font, score_string, &w, &h);
            SDL_Surface* score_surface = TTF_RenderUTF8_Blended(this->default_font, score_string,
                (SDL_Color){COLOR_WHITE});
            if(score_surface != NULL) {
                SDL_Texture* score = SDL_CreateTextureFromSurface(this->renderer, score_surface);
                SDL_RenderCopy(this->renderer, score, 
                    NULL, &(SDL_Rect){0, 160 * place + h, w, h});
                SDL_FreeSurface(score_surface);
                destroy_SDL_Texture(score);
            }
        }
    } else {
        // Hibás formátum esetén.
        int w, h;
        TTF_SizeUTF8(this->small_font, this->lboard_screen->syntax_error_msg, &w, &h);
        SDL_Surface* msg_surface = TTF_RenderUTF8_Blended(this->default_font, this->lboard_screen->syntax_error_msg,
            (SDL_Color){COLOR_WHITE});
        if(msg_surface != NULL) {
            SDL_Texture* error_msg = SDL_CreateTextureFromSurface(this->renderer, msg_surface);
            SDL_RenderCopy(this->renderer, error_msg, NULL, &(SDL_Rect){100, 100, w, h});
            SDL_FreeSurface(msg_surface);
            destroy_SDL_Texture(error_msg);
        }
    }

    SDL_SetRenderTarget(this->renderer, NULL);
    SDL_SetTextureBlendMode(this->lboard_screen->list_texture, SDL_BLENDMODE_BLEND);
}

/**
 * @brief Dicsőséglistanézet megjelenítése.
 *
 * @param this A `Carcassone` struktúra, amihez a dicsőséglista tartozik.
 */
void Carcassone__Lboard__render(Carcassone* this)
{
    SDL_SetRenderDrawColor(this->renderer, COLOR_BG);
    SDL_RenderClear(this->renderer);

    Carcassone__Button__render(this, &this->lboard_screen->back_button);

    SDL_RenderCopy(this->renderer, this->splash_title, NULL,
        &(SDL_Rect){this->width/2 - 400, 0, 800, 240});

    SDL_RenderCopy(this->renderer, this->lboard_screen->list_texture, NULL,
        &(SDL_Rect){this->width/2 - 500, 250, 1000, 700});
}

/**
 * @brief Létrehozza a játéknézetet.
 *
 * Megjegyzés: A lefoglalt memória megfelelő felszabadításához meg kell hívni a `Carcassone__Game__destroy` függvényt.
 *
 * @param this A `Carcassone` struktúra, amelynek létrehozza a játéknézetét.
 */
void Carcassone__Game__construct(Carcassone* this)
{
    this->game_screen = malloc(sizeof(GameScreen));
    this->game_screen->board_offset = (SDL_FPoint){this->width/2.0f-300, 120};
    this->game_screen->board = NULL;
    this->game_screen->active_input = NULL;
    this->game_screen->curr_player = NULL;
    this->game_screen->winner = NULL;
    this->game_screen->pile_index = 0U;
    this->game_screen->is_ready = false;
    this->game_screen->is_game_over = false;
    this->game_screen->drawn_tile = malloc(sizeof(Tile));
    this->game_screen->card_pile = CardPile__construct();
    for(size_t k = 0U; k < 4; ++k) {
        this->game_screen->held_arrow_keys[k] = 0;
    }

    // TODO: ezt ne így
    char counter_string[2] = "0";
    SDL_Surface* curr_index_surface = TTF_RenderText_Blended(this->default_font, counter_string, (SDL_Color){COLOR_WHITE});
    for(size_t ti = 0U; ti < PILE_SIZE; ++ti) {
        sprintf(counter_string, "%zu", PILE_SIZE - ti);
        curr_index_surface = TTF_RenderText_Blended(this->default_font, counter_string, (SDL_Color){COLOR_WHITE});
        if(curr_index_surface != NULL) {
            this->game_screen->pile_counter[ti] = SDL_CreateTextureFromSurface(this->renderer, curr_index_surface);
            SDL_FreeSurface(curr_index_surface);
        }

        if(this->game_screen->pile_counter[ti] == NULL)
            break;
    }

    this->game_screen->board_texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET, 600, this->height - this->game_screen->board_offset.y - 10);
    this->game_screen->tileset_wrapper = TilesetWrapper__construct(this->renderer);
    if(this->game_screen->tileset_wrapper.tile_set == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Nem lehetett betölteni az atlaszt!");
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "%s", SDL_GetError());
    }
    Carcassone__Game__init_board(this);
    Carcassone__Game__init_pile(this);
    Carcassone__Game__draw_new(this);

    int max_width, height;
    TTF_SizeUTF8(this->small_font, "WWWWWWWWWWWWWWWWWWWWWWWW", &max_width, &height);

    SDL_Rect input1_max_rect = {
        this->width / 2 - max_width / 2, this->height / 4 - 100, max_width, height + 10
    };
    SDL_Rect input2_max_rect = {
        this->width / 2 - max_width / 2, this->height / 4 + 150, max_width, height + 10
    };

    SDL_Surface* plabel1_surface = TTF_RenderUTF8_Blended(this->small_font, "Játékos 1", (SDL_Color){COLOR_LIGHTBLUE});
    if(plabel1_surface != NULL) {
        this->game_screen->player_input_labels[0] =
            SDL_CreateTextureFromSurface(this->renderer, plabel1_surface);
        SDL_FreeSurface(plabel1_surface);
    }
    SDL_Surface* plabel2_surface = TTF_RenderUTF8_Blended(this->small_font, "Játékos 2", (SDL_Color){COLOR_SALMON});
    if(plabel1_surface != NULL) {
        this->game_screen->player_input_labels[1] =
            SDL_CreateTextureFromSurface(this->renderer, plabel2_surface);
        SDL_FreeSurface(plabel2_surface);
    }

    this->game_screen->player_name_inputs[0] = 
        Carcassone__Prompt__construct(this, this->small_font, "", input1_max_rect, (SDL_Color){COLOR_WHITE}, (SDL_Color){0, 0, 0, 255});
    this->game_screen->player_name_inputs[1] = 
        Carcassone__Prompt__construct(this, this->small_font, "", input2_max_rect, (SDL_Color){COLOR_WHITE}, (SDL_Color){0, 0, 0, 255});
    this->game_screen->ready_button = 
        Carcassone__Button__construct(this, this->small_font, "OK", (SDL_Rect){10, this->height - 10 - 60, 150, 60}, (SDL_Color){COLOR_WHITE}, 
            (SDL_Color){0, 0, 0, 255}, true);
    this->game_screen->concede_button =
        Carcassone__Button__construct(this, this->small_font, "FELAD", (SDL_Rect){this->width - 10 - 150, 10, 150, 60}, (SDL_Color){COLOR_WHITE}, 
            (SDL_Color){140, 0, 0, 255}, true);
    this->game_screen->end_turn_button = // (SDL_Rect){this->width - 10 - 200, this->height - 10 - 60, 200, 60}
        Carcassone__Button__construct(this, this->small_font, "KÖR VÉGE", (SDL_Rect){10, 10, 200, 60}, (SDL_Color){COLOR_WHITE}, 
            (SDL_Color){0, 0, 140, 255}, true);

    this->game_screen->active_input = &this->game_screen->player_name_inputs[0];

    this->game_screen->crown_texture = create_SDL_texture_from_BMP(this->renderer, "./res/winners_crown.bmp");
}

/**
 * @brief Felszabadítja a megadott `Carcassone` struktúrához tartozó `GameScreen` által lefoglalt memóriát.
 *
 * @param this A `Carcassone` struktúra, aminek a lefoglalt memóriáját fel kell szabadítani.
 */
void Carcassone__Game__destroy(Carcassone* this)
{
    destroy_SDL_Texture(this->game_screen->board_texture);
    for(size_t n = 0U; n < PILE_SIZE; ++n) {
        destroy_SDL_Texture(this->game_screen->pile_counter[n]);
    }

    for(size_t p = 0U; p < 2; ++p) {
        Player__destroy(&this->game_screen->players[p]);
    }

    if(this->game_screen->board != NULL) {
        for(size_t n = 0U; n < BOARD_SIZE; ++n) {
            free(this->game_screen->board[n]);
        }
    }
    free(this->game_screen->board);
    CardPile__destroy(this->game_screen->card_pile);
    free(this->game_screen->drawn_tile);

    Carcassone__Button__destroy(this, &this->game_screen->ready_button);
    Carcassone__Button__destroy(this, &this->game_screen->concede_button);
    Carcassone__Button__destroy(this, &this->game_screen->end_turn_button);
    Carcassone__Prompt__destroy(this, &this->game_screen->player_name_inputs[0]);
    Carcassone__Prompt__destroy(this, &this->game_screen->player_name_inputs[1]);
    for(size_t i = 0U; i < 2; ++i) {
        destroy_SDL_Texture(this->game_screen->player_input_labels[i]);
    }

    destroy_SDL_Texture(this->game_screen->crown_texture);

    TilesetWrapper__destroy(&this->game_screen->tileset_wrapper);
    free(this->game_screen);
}

/**
 * @brief Játéknézet megjelenítése.
 *
 * @param this A `Carcassone` struktúra, amihez a nézet tartozik.
 */
void Carcassone__Game__render(Carcassone* this)
{
    SDL_SetRenderDrawColor(this->renderer, COLOR_BG);
    SDL_RenderClear(this->renderer);

    if(!this->game_screen->is_ready) {
        Carcassone__Prompt__render(this, &this->game_screen->player_name_inputs[0]);
        Carcassone__Prompt__render(this, &this->game_screen->player_name_inputs[1]);
        Carcassone__Button__render(this, &this->game_screen->ready_button);

        SDL_RenderCopy(this->renderer, this->game_screen->player_input_labels[0], NULL, &(SDL_Rect){
            this->width / 2 - 75, this->height / 4 - 170, 130, 50
        });
        SDL_RenderCopy(this->renderer, this->game_screen->player_input_labels[1], NULL, &(SDL_Rect){
            this->width / 2 - 75, this->height / 4 + 80, 130, 50
        });

        if(this->game_screen->active_input != NULL) {
            SDL_SetRenderDrawColor(this->renderer, COLOR_RED);
            SDL_RenderDrawRect(this->renderer, &this->game_screen->active_input->prompt.global_rect);
        }
    } else {
        Carcassone__Game__render_board(this);
        Carcassone__Game__render_meeples(this);
        if(!this->game_screen->is_game_over) {
            Carcassone__indicate_possible_placements(this);
            Carcassone__Game__render_drawn_tile(this);
        }
        Carcassone__render_splash_title(this);
        Carcassone__Game__render_player_stats(this);
        Carcassone__Button__render(this, &this->game_screen->concede_button);
        Carcassone__Button__render(this, &this->game_screen->end_turn_button);
        if(this->game_screen->is_game_over) {
            Carcassone__Game__render_game_over(this);
        }
    }
}