#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "game/player.h"
#include "game/tile.h"
#include "ui.h"
#include "app.h"

#ifdef _CRCLONE_DEBUG
    #include "debug/debugmalloc.h"
#endif

/**
 * @brief Initicializálja az összes nézetet, SDL és TTF kontextusokat.
 *
 * Megjegyzés: A lefoglalt memória megfelelő felszabadításához meg kell hívni a Carcassone__destroy(Carcassone*) függvényt.
 *
 * @param width Az ablak szélessége.
 * @param height Az ablak magassága.
 * @param title Az ablak címe.
 * @return Pointer az újonnan létrehozott Carcassone structra.
 */
Carcassone* Carcassone__construct(int width, int height, char const* title)
{
    Carcassone* new_app = malloc(sizeof(Carcassone));
    new_app->width = width;
    new_app->height = height;
    new_app->is_running = false;
    new_app->window = NULL;
    new_app->window_icon = NULL;
    new_app->splash_title = NULL;
    new_app->renderer = NULL;
    new_app->state = MENU;

    // Összes dolog betöltése
    if(SDL_Init(SDL_INIT_VIDEO) != 0 || TTF_Init() != 0) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Nem sikerült inicializálni az SDL2-t!");
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return NULL;
    }

    new_app->window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if(new_app->window == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Nem sikerült létrehozni az ablakot!");
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return NULL;
    }

    new_app->renderer = SDL_CreateRenderer(new_app->window, -1, SDL_RENDERER_ACCELERATED);
    if(new_app->renderer == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Nem sikerült létrehozni a renderert!");
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return NULL;
    }

    new_app->default_font = TTF_OpenFont("res/fonts/Sedan_SC/sedan_sc.ttf", 48);
    new_app->small_font = TTF_OpenFont("res/fonts/Sedan_SC/sedan_sc.ttf", 36);
    if(new_app->default_font == NULL || new_app->small_font == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Nem sikerült betölteni a fontot!");
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", TTF_GetError());
        return NULL;
    }

    // Nem gond, ha nincs (bár úgy jobban néz ki)
    new_app->splash_title = SDL_CreateTextureFromSurface(new_app->renderer, SDL_LoadBMP("res/splash_title.bmp"));
    if(new_app->splash_title == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Nem sikerült betölteni a címképet!");
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "%s", SDL_GetError());
    }

    // Nem gond, ha nincs
    new_app->window_icon = SDL_LoadBMP("res/crc_icon.bmp");
    if(new_app->window_icon != NULL) {
        SDL_SetWindowIcon(new_app->window, new_app->window_icon);
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Nem sikerült betölteni az appikont!");
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
    }

    srand((unsigned int)(time(NULL) % SHRT_MAX));

    Carcassone__Menu__construct(new_app);
    Carcassone__Game__construct(new_app);
    Carcassone__Lboard__construct(new_app);

    SDL_StartTextInput();

    new_app->smutex = SDL_CreateMutex();

    return new_app;
}

/**
 * @brief Felszabadítja a megadott Carcassone struktúra által lefoglalt memóriát.
 *
 * @param this A Carcassone struktúra, aminek a lefoglalt memóriáját fel kell szabadítani.
 */
void Carcassone__destroy(Carcassone* this)
{
    SDL_UnlockMutex(this->smutex);
    SDL_DestroyMutex(this->smutex);
    this->is_running = false;
    if(SDL_IsTextInputActive()) SDL_StopTextInput();

    Carcassone__Menu__destroy(this);
    Carcassone__Game__destroy(this);
    Carcassone__Lboard__destroy(this);

    if(this->window_icon != NULL)   SDL_FreeSurface(this->window_icon);
    if(this->splash_title != NULL)  SDL_DestroyTexture(this->splash_title);
    if(this->window != NULL)        SDL_DestroyWindow(this->window);
    if(this->renderer != NULL)      SDL_DestroyRenderer(this->renderer);
    if(this->default_font != NULL)  TTF_CloseFont(this->default_font);
    if(TTF_WasInit() != 0)          TTF_Quit();
    if(SDL_WasInit(0) != 0)   SDL_Quit();

    free(this);
}

void Carcassone__switch_state(Carcassone* this, AppState new_state)
{
    if(this->state == new_state) return;

    this->state = new_state;
    switch(new_state) {
        case MENU:
            this->game_screen->is_ready = false;
            this->game_screen->is_game_over = false;
            break;
        case GAME:
            this->game_screen->is_ready = false;
            this->game_screen->is_game_over = false;
            Carcassone__Game__init_board(this);
            Carcassone__Game__init_pile(this);
            break;
        case LEADERBOARD:
            Carcassone__Lboard__reconstruct(this);
            break;
    }
}


static void remove_last_utf8_char(char* str) {
    if(str == NULL) return;
    size_t len = strlen(str);
    if (len == 0) return;

    char* p = (char*)str + len - 1;

    // UTF-8 enkódolási szabályok:
    // - 1-byte karakter: 0xxxxxxx
    // - 2-byte karakter: 110xxxxx 10xxxxxx
    // - 3-byte karakter: 1110xxxx 10xxxxxx 10xxxxxx
    // - 4-byte karakter: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

    while (p >= (char*)str) {
        // Megkeressük az első, nem "continuation" bitet; odatesszük a \0-t.
        if ((*p & 0b11000000) != 0b10000000) {
            *p = '\0';
            break;
        }
        --p;
    }
}

size_t get_utf8_length(char* str)
{
    if(str == NULL) return 0U;

    char* p = str;
    size_t len = 0U;
    while(*p != '\0') {
        if ((*p & 0b11000000) != 0b10000000) {
            ++len;
        }
        ++p;
    }

    return len;
}

/**
 * @brief Inputok kezelése.
 * 
 * SDL_Event-öket kezel.
 *
 * @param this A Carcassone struktúra, ami tartalmazza az SDL kontextust.
 */
void Carcassone__handle_input(Carcassone* this)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch(event.type) {
        case SDL_QUIT:
            this->is_running = false;
            break;
        case SDL_TEXTINPUT:
            if(!(this->state == GAME && !this->game_screen->is_ready)) break;
            if(event.text.text[0] == ' ') break;

            if(get_utf8_length(this->game_screen->active_input->prompt.label) < 24) {
                Carcassone__Prompt__edit(this, this->game_screen->active_input, event.text.text, true);
            }
            break;
        case SDL_TEXTEDITING:
            break;
        case SDL_KEYUP:
            if(!(SDL_SCANCODE_RIGHT <= event.key.keysym.scancode && event.key.keysym.scancode <= SDL_SCANCODE_UP)) return;

            unsigned int key = event.key.keysym.scancode - SDL_SCANCODE_RIGHT;
            this->game_screen->held_arrow_keys[key] = false;

            break;
        case SDL_KEYDOWN:
            switch(event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    this->is_running = false;
                    break;
                case SDLK_BACKSPACE:
                    if(get_utf8_length(this->game_screen->active_input->prompt.label) > 0) {
                        remove_last_utf8_char(this->game_screen->active_input->prompt.label);
                        Carcassone__Prompt__edit(this, this->game_screen->active_input, this->game_screen->active_input->prompt.label, false);
                    }
                    break;
                case SDLK_r:
                    if(this->game_screen->is_ready) Tile__rotate(this->game_screen->drawn_tile);
                    break;
                case SDLK_d: // TODO: temp
                    if(this->game_screen->is_game_over) break;
                    Carcassone__Game__draw_new(this);
                    Carcassone__check_scorable_constructs(this);
                    break;
                default:
                    if(!(SDL_SCANCODE_RIGHT <= event.key.keysym.scancode && event.key.keysym.scancode <= SDL_SCANCODE_UP)) break;

                    unsigned int key = event.key.keysym.scancode - SDL_SCANCODE_RIGHT;
                    this->game_screen->held_arrow_keys[key] = true;

                    break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if(this->state == MENU) {
                if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->menu_screen->start_button.global_rect)) {
                    Carcassone__switch_state(this, GAME);
                }
                if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->menu_screen->lboard_button.global_rect)) {
                    Carcassone__switch_state(this, LEADERBOARD);
                }
            }

            if(this->state == LEADERBOARD) {
                if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->lboard_screen->back_button.global_rect)) {
                    Carcassone__switch_state(this, MENU);
                }
            }

            if(this->state != GAME) break;

            if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->game_screen->player_name_inputs[0].prompt.global_rect)) {
                this->game_screen->active_input = &this->game_screen->player_name_inputs[0];
            } else if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->game_screen->player_name_inputs[1].prompt.global_rect)) {
                this->game_screen->active_input = &this->game_screen->player_name_inputs[1];
            }

            if(!this->game_screen->is_ready) {
                if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->game_screen->ready_button.global_rect)) {
                    if(!Carcassone__check_names_valid(this)) break;
                    this->game_screen->is_ready = true;
                    Carcassone__Game__init_players(this);
                }
            } else {
                if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->game_screen->end_turn_button.global_rect)) {
                    if(this->game_screen->curr_player->has_placed_card) {
                        Carcassone__Game__draw_new(this);
                        Carcassone__check_scorable_constructs(this);
                    }
                }
                if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->game_screen->concede_button.global_rect))
                    Carcassone__Game__show_finish_screen(this);
                
                for(int y = 0; y < BOARD_SIZE; ++y) {
                    for(int x = 0; x < BOARD_SIZE; ++x) {
                        Tile* curr_tile = &this->game_screen->board[x][y];
                        if(!this->game_screen->curr_player->has_placed_card) {
                            if(Tile__point_in_tile(curr_tile, (SDL_FPoint){event.button.x, event.button.y})
                                    && curr_tile->type == EMPTY
                                    && Carcassone__check_surrounding_tiles(this, (SDL_Point){x, y})
                            ) {
                                Tile__set_type(curr_tile, this->game_screen->drawn_tile->type, this->game_screen->drawn_tile->rotation);
                                // Tile__set_rotation(curr_tile, this->game_screen->drawn_tile->rotation);

                                this->game_screen->curr_player->has_placed_card = true;
                            }
                        } else { 
                            if(Tile__point_in_tile(curr_tile, (SDL_FPoint){event.button.x, event.button.y}) 
                                    && curr_tile->type != EMPTY && !curr_tile->is_scored && !curr_tile->is_expired) {
                                Player__place_meeple(this->game_screen->curr_player, (SDL_Point){x, y});
                            }
                        }
                    }
                }
            }

            break;
        default:
            break;
    }

    if(this->game_screen->is_ready) Carcassone__Game__move_board(this, 0U);
}

bool Carcassone__check_names_valid(Carcassone* this)
{
    for(size_t p = 0U; p < 2; ++p) {
        if(strlen(this->game_screen->player_name_inputs[p].prompt.label) == 0) return false;
    }

    if(strcmp(this->game_screen->player_name_inputs[0].prompt.label, this->game_screen->player_name_inputs[1].prompt.label) == 0) return false;

    return true;
}

/**
 * @brief Játékosok létrehozása.
 * 
 * Inicializálja a két játékost előre megadott adatok alapján (ezek a GameScreenben találhatók).
 *
 * @param this A Carcassone struktúra.
 */
void Carcassone__Game__init_players(Carcassone* this) // TODO
{
    this->game_screen->players[0] = Player__construct(this->renderer, this->small_font,
            this->game_screen->player_name_inputs[0].prompt.label); // TODO: NO
    this->game_screen->players[1] = Player__construct(this->renderer, this->small_font,
            this->game_screen->player_name_inputs[1].prompt.label); // TODO: NO

    this->game_screen->curr_player = &this->game_screen->players[0];
}

/**
 * @brief Kártyapakli létrehozása.
 * 
 * Véletlenszerűen megkeveri a paklit és létrehozza a mezőkártyákat mindegyikhez.
 *
 * @param this A Carcassone struktúra.
 */
void Carcassone__Game__init_pile(Carcassone* this)
{
    // ? TODO: put this in a config file
    TileType pile[PILE_SIZE] = {
        FIELD_CLOISTER_ROAD_S, FIELD_CLOISTER_ROAD_S,
        FIELD_CLOISTER_ROAD_S, FIELD_CLOISTER_ROAD_S,
        FIELD_CLOISTER_ROAD_NS, FIELD_CLOISTER_ROAD_NS,
        FIELD_VILLAGE_ROAD_S, FIELD_VILLAGE_ROAD_S,
        FIELD_VILLAGE_ROAD_NS, FIELD_VILLAGE_ROAD_NS,
        ROAD_NS, ROAD_NS, ROAD_NS, ROAD_NS,
        ROAD_NS, ROAD_NS, ROAD_NS, ROAD_NS,
        ROAD_NW, ROAD_NW, ROAD_NW, ROAD_NW,
        ROAD_NW, ROAD_NW, ROAD_NW, ROAD_NW,
        ROAD_NW,
        ROAD_NWE, ROAD_NWE, ROAD_NWE, ROAD_NWE,
        ROAD_NSWE,
        CASTLE_CAP_WALL, CASTLE_CAP_WALL,
        CASTLE_CAP_WALL, CASTLE_CAP_WALL,
        CASTLE_CAP_WALL,
        CASTLE_CAP_WALL_ROAD_BY, CASTLE_CAP_WALL_ROAD_BY,
        CASTLE_CAP_WALL_ROAD_BY, CASTLE_CAP_WALL_ROAD_BY,
        CASTLE_CAP_WALL_ROAD_BY, CASTLE_CAP_WALL_ROAD_BY,
        CASTLE_CAP_WALL_ROAD_BY, CASTLE_CAP_WALL_ROAD_BY,
        CASTLE_CAP_WALL_ROAD_TO, CASTLE_CAP_WALL_ROAD_TO,
        CASTLE_CAP_WALL_ROAD_TO,
        CASTLE_CORNER_WALL_ROAD_BY, CASTLE_CORNER_WALL_ROAD_BY,
        CASTLE_CORNER_WALL_ROAD_BY, CASTLE_CORNER_WALL_ROAD_BY,
        CASTLE_CORNER_WALL, CASTLE_CORNER_WALL,
        CASTLE_CORNER_WALL, CASTLE_CORNER_WALL,
        CASTLE_TOWN, CASTLE_TOWN, CASTLE_TOWN,
        CASTLE_PANTHEON, CASTLE_PANTHEON,
        CASTLE_TUNNEL, CASTLE_TUNNEL,
        CASTLE_TUNNEL, CASTLE_TUNNEL,
        CASTLE_SHIRT_WALL, CASTLE_SHIRT_WALL,
        CASTLE_SHIRT_WALL, CASTLE_SHIRT_WALL,
        CASTLE_SHIRT_WALL_ROAD_TO, CASTLE_SHIRT_WALL_ROAD_TO
    };

    for(size_t n = 0U; n < PILE_SIZE; ++n) {
        size_t rand_index = (size_t) (rand() % PILE_SIZE); // SIZE_MAX > INT_MAX (legtöbb esetben, de ha nem akkor meg wrappel)
        TileType temp = pile[n];

        pile[n] = pile[rand_index];
        pile[rand_index] = temp;
    }

    for(size_t n = 0U; n < PILE_SIZE; ++n) {
        this->game_screen->card_pile = CardPile__push(this->game_screen->card_pile, pile[n]);
    }

    this->game_screen->pile_index = 0U;
}

/**
 * @brief Játéktábla létrehozása.
 * 
 * Létrehozza a játéktáblát és le is helyezi a kezdőkártyát.
 *
 * @param this A Carcassone struktúra.
 */
void Carcassone__Game__init_board(Carcassone* this)
{
    // BOARD_SIZE * BOARD_SIZE 2D array
    // ! debugmalloc falsely claims the code below produces a memory leak
    // ! it does not, free happens inside Carcassone__Game__destroy() without any issues
    this->game_screen->board = malloc(BOARD_SIZE * sizeof(Tile*));
    for(size_t n = 0U; n < BOARD_SIZE; ++n) {
        this->game_screen->board[n] = malloc(BOARD_SIZE * sizeof(Tile));
    }

    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
            Tile__construct(&this->game_screen->board[x][y], EMPTY, (SDL_Point){x, y}, this->game_screen->board_offset);
            Tile__move_by(&this->game_screen->board[x][y], 
                -BOARD_SIZE / 2.0f + 3.0f, 
                -BOARD_SIZE / 2.0f + 4.0f);
        }
    }

    // Kezdőkártya
    Tile__construct(&this->game_screen->board[BOARD_SIZE / 2][BOARD_SIZE / 2], CASTLE_CAP_WALL_ROAD_BY,
        (SDL_Point){BOARD_SIZE / 2, BOARD_SIZE / 2}, this->game_screen->board_offset);
    Tile__move_by(&this->game_screen->board[BOARD_SIZE / 2][BOARD_SIZE / 2], 
                -BOARD_SIZE / 2.0f + 3.0f, 
                -BOARD_SIZE / 2.0f + 4.0f);
}

/**
 * @brief Játéktábla renderelése.
 * 
 * @param this A Carcassone struktúra, ami tartalmazza az SDL kontextust.
 */
void Carcassone__Game__render_board(Carcassone* this)
{
    SDL_Rect viewport_rect = 
        {this->game_screen->board_offset.x, this->game_screen->board_offset.y, 600, this->height - this->game_screen->board_offset.y - 10};
    SDL_SetRenderTarget(this->renderer, this->game_screen->board_texture);
    SDL_RenderClear(this->renderer);

    for(int y = 0U; y < BOARD_SIZE; ++y) {
        for(int x = 0U; x < BOARD_SIZE; ++x) {
            Tile* curr_tile = &this->game_screen->board[x][y];
            SDL_Rect tile_rect = {
                curr_tile->local_coords.x,
                curr_tile->local_coords.y,
                TILE_SIZE, TILE_SIZE
            };

            SDL_Rect ts_rect = get_texture_rect_for(curr_tile->type);
            SDL_RenderCopyEx(this->renderer, this->game_screen->tileset_wrapper.tile_set, &ts_rect, &tile_rect,
                curr_tile->rotation, NULL, SDL_FLIP_NONE);

            // Fehér keret a celláknak
            SDL_SetRenderDrawColor(this->renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(this->renderer, &tile_rect);
        }
    }

    SDL_SetRenderTarget(this->renderer, NULL);
    SDL_RenderCopy(this->renderer, this->game_screen->board_texture, NULL, &viewport_rect);
}

/**
 * @brief A splash cím renderelése.
 * 
 * @param this A Carcassone struktúra, ami tartalmazza az SDL kontextust.
 */
void Carcassone__render_splash_title(Carcassone* this)
{
    SDL_RenderCopy(this->renderer, this->splash_title, NULL,
        &(SDL_Rect){this->game_screen->board_offset.x, 0, 400, 120});
}

/**
 * @brief A pakli tetején levő kártya renderelése.
 * 
 * @param this A Carcassone struktúra, ami tartalmazza az SDL kontextust.
 */
void Carcassone__Game__render_drawn_tile(Carcassone* this)
{
    SDL_Rect ts_rect = get_texture_rect_for(this->game_screen->drawn_tile->type);

    SDL_RenderCopyEx(this->renderer, this->game_screen->tileset_wrapper.tile_set, &ts_rect,
        &(SDL_Rect){this->game_screen->board_offset.x + 450, this->game_screen->board_offset.y - TILE_SIZE, TILE_SIZE-5, TILE_SIZE-5},
        this->game_screen->drawn_tile->rotation, NULL, SDL_FLIP_NONE);

    SDL_Rect pile_index_rect = {this->game_screen->board_offset.x + 555, this->game_screen->board_offset.y - TILE_SIZE/2.0f, 50, 50};
    if(PILE_SIZE - this->game_screen->pile_index < 10) {
        pile_index_rect.w /= 2;
        pile_index_rect.x += pile_index_rect.w/2;
    }

    SDL_RenderCopy(this->renderer, this->game_screen->pile_counter[this->game_screen->pile_index], NULL, &pile_index_rect);

    // TODO: temp
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    if(!this->game_screen->curr_player->has_placed_card) {
        SDL_RenderCopyEx(this->renderer, this->game_screen->tileset_wrapper.tile_set, &ts_rect,
            &(SDL_Rect){mx - TILE_SIZE / 2, my - TILE_SIZE / 2, TILE_SIZE, TILE_SIZE},
            this->game_screen->drawn_tile->rotation, NULL, SDL_FLIP_NONE);
    } else if(this->game_screen->curr_player->meeples_at_hand > 0) {
        SDL_RenderCopy(this->renderer, this->game_screen->curr_player->meeples[0].texture, NULL, 
            &(SDL_Rect){mx - 64 / 2, my - 64 / 2, 64, 64});
    }
}

void Carcassone__Game__render_meeples(Carcassone* this)
{
    SDL_Rect rect;
    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
            Tile* placed_on = &this->game_screen->board[x][y];
            unsigned int offset_f = 0U;
            unsigned int offset_b = 0U;
            for(size_t p = 0U; p < 2; ++p) {
                for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
                    Meeple* curr_m = &this->game_screen->players[p].meeples[m];
                    if(!curr_m->is_placed) continue;
                    if(curr_m->x != placed_on->board_coords.x || curr_m->y != placed_on->board_coords.y) continue;

                    rect = (SDL_Rect){
                        p == 0 ? placed_on->global_coords.x + 10*offset_f : placed_on->global_coords.x + TILE_SIZE - 32 - 10*offset_b,
                        p == 0 ? placed_on->global_coords.y : placed_on->global_coords.y + TILE_SIZE - 32,
                        32, 32
                    };
                    SDL_RenderCopy(this->renderer, curr_m->texture, NULL, &rect);
                    if(p == 0) ++offset_f;
                    else ++offset_b;
                }
            }
        }
    }
}

void Carcassone__Game__render_game_over(Carcassone* this)
{
    if(this->game_screen->winner == NULL) return;

    SDL_SetRenderDrawColor(this->renderer, COLOR_RED);
    if(this->game_screen->winner == &this->game_screen->players[0]) {
        SDL_RenderDrawRect(this->renderer, &(SDL_Rect){10, this->height - 710, 300, 700});
        SDL_RenderCopy(this->renderer, this->game_screen->crown_texture, NULL, 
            &(SDL_Rect){150 - 64, this->height - 710 - 128, 128, 128});
    } else {
        SDL_RenderDrawRect(this->renderer, &(SDL_Rect){this->width - 310, this->height - 710, 300, 700});
        SDL_RenderCopy(this->renderer, this->game_screen->crown_texture, NULL, 
            &(SDL_Rect){this->width - 310/2 - 64, this->height - 710 - 128, 128, 128});
    }
}

void Carcassone__indicate_possible_placements(Carcassone* this)
{
    if(this->game_screen->curr_player->has_placed_card) return;
    
    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
            Tile* curr_tile = &this->game_screen->board[x][y];
            if(curr_tile->type != EMPTY) continue;

            if(Carcassone__check_surrounding_tiles(this, (SDL_Point){x, y})) {
                SDL_SetRenderDrawColor(this->renderer, 20, 240, 100, 255);
                SDL_RenderFillRect(this->renderer, 
                &(SDL_Rect){
                        curr_tile->global_coords.x, curr_tile->global_coords.y, TILE_SIZE, TILE_SIZE}
                );
            }
        }
    }
}

bool Carcassone__check_if_possible(Carcassone* this)
{
    if(this->game_screen->drawn_tile == NULL) return false;

    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
            Tile* curr_tile = &this->game_screen->board[x][y];
            if(curr_tile->type != EMPTY) continue;

            for(unsigned int rot = 0U; rot < 4; ++rot) {
                if(Carcassone__check_surrounding_tiles(this, (SDL_Point){x, y})) {
                    return true;
                }

                if(!this->game_screen->drawn_tile->rotatable) break;
                else Tile__rotate(this->game_screen->drawn_tile);
            }

        }
    }

    return false;
}

void Carcassone__check_scorable_constructs(Carcassone* this)
{
    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
            Tile* curr_tile = &this->game_screen->board[x][y];
            if(curr_tile->type == EMPTY || curr_tile->is_scored) continue;

            if(!this->game_screen->is_game_over) {
                if(curr_tile->type == FIELD_CLOISTER_ROAD_S || curr_tile->type == FIELD_CLOISTER_ROAD_NS) {
                    curr_tile->is_expired = true;
                    bool finished = true;
                    for(int yrel = y-1; yrel <= y+1; ++yrel) {
                        for(int xrel = x-1; xrel <= x+1; ++xrel) {
                            if(yrel < 0 || yrel >= BOARD_SIZE || xrel < 0 || xrel >= BOARD_SIZE) continue;
                            if(this->game_screen->board[xrel][yrel].type == EMPTY) {
                                finished = false;
                                break;
                            }
                        }

                        if(!finished) break;
                    }

                    if(finished) {
                        Carcassone__calculate_scores_for_cloister(this, curr_tile);
                    }
                }
            }

            // WIP
            bool has_road_out = false;
            for(unsigned int dir = NORTH; dir <= WEST; ++dir) {
                if(curr_tile->connections[dir] == ROAD) {
                    has_road_out = true;
                    break;
                }
            }

            if(has_road_out) {
                bool finished = true;
                unsigned int num_of_connected_roads = 0U;

                SDL_Point dir_rel_coords[4] = {
                    {0, -1}, {1, 0}, {0, 1}, {-1, 0}
                };

                // TODO: NO
                size_t visited_idx = 0U;
                Tile* visited[PILE_SIZE] = {0};
                size_t stack_idx = 0U;
                Tile* stack[PILE_SIZE] = {0};
                
                stack[stack_idx] = curr_tile;
                ++stack_idx;
                while(stack_idx != 0U) {
                    --stack_idx;
                    Tile* popped = stack[stack_idx];
                    if(!this->game_screen->is_game_over && popped->type == EMPTY) {
                        finished = false;
                        break;
                    }
                    
                    bool is_visited = false;
                    for(size_t vi = 0U; vi < visited_idx; ++vi) {
                        if(popped->board_coords.x == visited[vi]->board_coords.x && popped->board_coords.y == visited[vi]->board_coords.y) {
                            is_visited = true;
                            break;
                        }
                    }
                    if(is_visited) {
                        continue;
                    } else {
                        if(popped->type != EMPTY) ++num_of_connected_roads;
                        if(0 <= popped->type && popped->type <= 3) ++num_of_connected_roads;
                        visited[visited_idx] = popped;
                        ++visited_idx;
                    }

                    for(unsigned int dir = NORTH; dir <= WEST; ++dir) {
                        int new_x = popped->board_coords.x + dir_rel_coords[dir].x;
                        int new_y = popped->board_coords.y + dir_rel_coords[dir].y;
                        if(new_x < 0 || new_x >= BOARD_SIZE || new_y < 0 || new_y >= BOARD_SIZE) continue;

                        if(popped->connections[dir] == ROAD) {
                            stack[stack_idx] = &this->game_screen->board[new_x][new_y];
                            ++stack_idx;
                        }

                    }

                    if(!finished) break;
                }

                if(finished) {
                    Carcassone__calculate_scores_for_road(this, visited, visited_idx, num_of_connected_roads);
                    for(size_t i = 0U; i < visited_idx; ++i) {
                        visited[i]->is_scored = true;
                    }
                }
            }
            // WIP

            // WIP
            bool has_castle_out = false;
            for(unsigned int dir = NORTH; dir <= WEST; ++dir) {
                if(curr_tile->connections[dir] == CASTLE) {
                    has_castle_out = true;
                    break;
                }
            }

            if(has_castle_out) {
                bool finished = true;
                unsigned int num_of_connected_castles = 0U;

                SDL_Point dir_rel_coords[4] = {
                    {0, -1}, {1, 0}, {0, 1}, {-1, 0}
                };

                // TODO: NO
                size_t visited_idx = 0U;
                Tile* visited[PILE_SIZE] = {0};
                size_t stack_idx = 0U;
                Tile* stack[PILE_SIZE] = {0};
                
                stack[stack_idx] = curr_tile;
                ++stack_idx;
                while(stack_idx != 0U) {
                    --stack_idx;
                    Tile* popped = stack[stack_idx];
                    if(!this->game_screen->is_game_over && popped->type == EMPTY) {
                        finished = false;
                        break;
                    }
                    
                    bool is_visited = false;
                    for(size_t vi = 0U; vi < visited_idx; ++vi) {
                        if(popped->board_coords.x == visited[vi]->board_coords.x && popped->board_coords.y == visited[vi]->board_coords.y) {
                            is_visited = true;
                            break;
                        }
                    }
                    if(is_visited) {
                        continue;
                    } else {
                        if(popped->type != EMPTY) ++num_of_connected_castles;
                        if(8 <= popped->type && popped->type <= 9) ++num_of_connected_castles;
                        visited[visited_idx] = popped;
                        ++visited_idx;
                    }

                    for(unsigned int dir = NORTH; dir <= WEST; ++dir) {
                        int new_x = popped->board_coords.x + dir_rel_coords[dir].x;
                        int new_y = popped->board_coords.y + dir_rel_coords[dir].y;
                        if(new_x < 0 || new_x >= BOARD_SIZE || new_y < 0 || new_y >= BOARD_SIZE) continue;

                        if(popped->connections[dir] == CASTLE) {
                            stack[stack_idx] = &this->game_screen->board[new_x][new_y];
                            ++stack_idx;
                        }

                    }

                    if(!finished) break;
                }

                if(finished) {
                    Carcassone__calculate_scores_for_castle(this, visited, visited_idx, num_of_connected_castles);
                    for(size_t i = 0U; i < visited_idx; ++i) {
                        visited[i]->is_scored = true;
                    }
                }
            }
            // WIP
        }
    }
}

// TODO: NO
void Carcassone__calculate_scores_for_cloister(Carcassone* this, Tile* cloister)
{
    for(size_t p = 0U; p < 2; ++p) {
        for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
            Meeple* curr_m = &this->game_screen->players[p].meeples[m];
            if(!curr_m->is_placed) continue;
            if(curr_m->x != cloister->board_coords.x || curr_m->y != cloister->board_coords.y) continue;

            Player__add_to_score(&this->game_screen->players[p], this->renderer, this->small_font, 3);
            Player__reclaim_meeple(&this->game_screen->players[p], curr_m);
            cloister->is_scored = true;
            DBG_LOG("Added score (cloister) to: %s", this->game_screen->players[p].name);
        }
    }
}

// TODO
void Carcassone__calculate_scores_for_road(Carcassone* this, Tile** roads, size_t num, unsigned int point)
{
    for(size_t i = 0U; i < num; ++i) {
        for(size_t p = 0U; p < 2; ++p) {
            for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
                Meeple* curr_m = &this->game_screen->players[p].meeples[m];
                if(!curr_m->is_placed) continue;
                if(curr_m->x != roads[i]->board_coords.x || curr_m->y != roads[i]->board_coords.y) continue;

                Player__add_to_score(&this->game_screen->players[p], this->renderer, this->small_font, point);
                Player__reclaim_meeple(&this->game_screen->players[p], curr_m);
                roads[i]->is_scored = true;
                DBG_LOG("Added score (road) to: %s", this->game_screen->players[p].name);
            }
        }
    }
}

// TODO
void Carcassone__calculate_scores_for_castle(Carcassone* this, Tile** castles, size_t num, unsigned int point)
{
    for(size_t i = 0U; i < num; ++i) {
        for(size_t p = 0U; p < 2; ++p) {
            for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
                Meeple* curr_m = &this->game_screen->players[p].meeples[m];
                if(!curr_m->is_placed) continue;
                if(curr_m->x != castles[i]->board_coords.x || curr_m->y != castles[i]->board_coords.y) continue;

                Player__add_to_score(&this->game_screen->players[p], this->renderer, this->small_font, point);
                Player__reclaim_meeple(&this->game_screen->players[p], curr_m);
                castles[i]->is_scored = true;
                DBG_LOG("Added score (castle) to: %s", this->game_screen->players[p].name);
            }
        }
    }
}

/**
 * @brief A splash cím renderelése.
 * 
 * @param this A Carcassone struktúra, ami tartalmazza az SDL kontextust.
 */
void Carcassone__Game__render_player_stats(Carcassone* this)
{   
    if(this->game_screen->players[0].stat_panel != NULL) {
        Player__render(&this->game_screen->players[0], this->renderer, this->small_font);
        SDL_RenderCopy(this->renderer, this->game_screen->players[0].stat_panel, NULL, 
            &(SDL_Rect){10, this->height - 710, 300, 700});
        if(this->game_screen->curr_player == &this->game_screen->players[0]) {
            SDL_SetRenderDrawColor(this->renderer, COLOR_SALMON);
            SDL_RenderDrawRect(this->renderer, &(SDL_Rect){10, this->height - 710, 300, 700});
        }
    }

    if(this->game_screen->players[1].stat_panel != NULL) {
        Player__render(&this->game_screen->players[1], this->renderer, this->small_font);
        SDL_RenderCopy(this->renderer, this->game_screen->players[1].stat_panel, NULL, 
            &(SDL_Rect){this->width - 310, this->height - 710, 300, 700});
        if(this->game_screen->curr_player == &this->game_screen->players[1]) {
            SDL_SetRenderDrawColor(this->renderer, COLOR_SALMON);
            SDL_RenderDrawRect(this->renderer, &(SDL_Rect){this->width - 310, this->height - 710, 300, 700});
        }
    }
}

/**
 * @brief Játéktábla mozgatása.
 * 
 * A nyilak segítségével a játéktábla látható részét mozgatja.
 *
 * @param this A Carcassone struktúra, ami tartalmazza az SDL kontextust.
 * @param key A beolvasott billentyű (a Carcassone__handle_inputs(Carcassone*)-tól kapja meg).
 */
void Carcassone__Game__move_board(Carcassone* this, SDL_Scancode key)
{
    float mvx = 0.0f;
    float mvy = 0.0f;

    // TODO: should depend on dt
    if(this->game_screen->held_arrow_keys[0]) {
        mvx -= 0.02f;
    }
    if(this->game_screen->held_arrow_keys[1]) {
        mvx += 0.02f;
    }
    if(this->game_screen->held_arrow_keys[2]) {
        mvy -= 0.02f;
    }
    if(this->game_screen->held_arrow_keys[3]) {
        mvy += 0.02f;
    }

    for(int y = 0U; y < BOARD_SIZE; ++y) {
        for(int x = 0U; x < BOARD_SIZE; ++x) {
            Tile__move_by(&this->game_screen->board[x][y], mvx, mvy);
        }
    }
}

/**
 * @brief Helyes pozíció ellenőrzése.
 * 
 * @param this A Carcassone struktúra, ami tartalmazza az SDL kontextust.
 * @param tcoords A kapott kártya potenciális helye a táblán.
 * @return Letehető-e a megfelelő pozícióba az adott kártya.
 */
bool Carcassone__check_surrounding_tiles(Carcassone* this, SDL_Point tcoords)
{
    int x = tcoords.x;
    int y = tcoords.y;

    // TODO: for loop
    // még egy korábbi refaktor előtti állapotból maradt így, amikor nem lehetett volna ciklussal megoldani
    bool next_to_placed = false;
    if(y - 1 >= 0) {
        if(this->game_screen->board[x][y-1].type != EMPTY) next_to_placed = true;
        if(this->game_screen->board[x][y-1].connections[SOUTH] != this->game_screen->drawn_tile->connections[NORTH] 
            && this->game_screen->board[x][y-1].connections[SOUTH] != NONE) {
            return false;
        }
    }
    if(y + 1 < BOARD_SIZE) {
        if(this->game_screen->board[x][y+1].type != EMPTY) next_to_placed = true;
        if(this->game_screen->board[x][y+1].connections[NORTH] != this->game_screen->drawn_tile->connections[SOUTH]
            && this->game_screen->board[x][y+1].connections[NORTH] != NONE) {
            return false;
        }
    }
    if(x - 1 >= 0) {
        if(this->game_screen->board[x-1][y].type != EMPTY) next_to_placed = true;
        if(this->game_screen->board[x-1][y].connections[EAST] != this->game_screen->drawn_tile->connections[WEST] 
            && this->game_screen->board[x-1][y].connections[EAST] != NONE) {
            return false;
        }
    }
    if(x + 1 < BOARD_SIZE) {
        if(this->game_screen->board[x+1][y].type != EMPTY) next_to_placed = true;
        if(this->game_screen->board[x+1][y].connections[WEST] != this->game_screen->drawn_tile->connections[EAST] 
            && this->game_screen->board[x+1][y].connections[WEST] != NONE) {
            return false;
        }
    }

    return next_to_placed;

}

/**
 * @brief Új kártya húzása a pakli tetejéről.
 * 
 * Húz egy új kártyát, ha kifogyott akkor kilép a menübe (egyelőre).
 *
 * @param this A Carcassone struktúra.
 */
void Carcassone__Game__draw_new(Carcassone* this) // TODO
{
    TileType next_type;
    this->game_screen->card_pile = CardPile__pop(this->game_screen->card_pile, &next_type);
    if(this->game_screen->card_pile != NULL) {
        Tile__construct(this->game_screen->drawn_tile, next_type, (SDL_Point){-1, -1}, (SDL_FPoint){0, 0});
        ++this->game_screen->pile_index;

        if(this->game_screen->curr_player != NULL) {
            this->game_screen->curr_player->has_placed_card = false;
        }

        // TODO: NO
        if(this->game_screen->curr_player->name == this->game_screen->players[0].name) {
            this->game_screen->curr_player = &this->game_screen->players[1];
        } else {
            this->game_screen->curr_player = &this->game_screen->players[0];
        }
    } else {

    }

    if(this->game_screen->pile_index >= PILE_SIZE) {
        Carcassone__Game__show_finish_screen(this);
    } else if(!this->game_screen->curr_player->has_placed_card && !Carcassone__check_if_possible(this)) {
        Carcassone__Game__draw_new(this);
        DBG_LOG("Skipped a card.");
    }

}

static unsigned int return_to_menu(unsigned int interval, void* this)
{  
    Carcassone* thiz = (Carcassone*) this;
    SDL_LockMutex(thiz->smutex);
    Carcassone__switch_state(thiz, MENU);
    SDL_UnlockMutex(thiz->smutex);
    return 0;
}

void Carcassone__Game__show_finish_screen(Carcassone* this) // TODO: rename to "wrap up" or some shi
{
    this->game_screen->is_game_over = true;
    Carcassone__check_scorable_constructs(this);

    if(this->game_screen->players[0].score == this->game_screen->players[1].score) {
        this->game_screen->winner = NULL;
    } else {
        this->game_screen->winner = 
            this->game_screen->players[0].score > this->game_screen->players[1].score ? &this->game_screen->players[0] : &this->game_screen->players[1];
    }
    if(this->lboard_screen->leaderboard != NULL) {
        Leaderboard__insert_new(this->lboard_screen->leaderboard, this->game_screen->winner);
        Leaderboard__load(this->lboard_screen->leaderboard, "res/data/records.dat");
    }

    DBG_LOG("The winner is: %s", this->game_screen->winner == NULL ? "tie" : this->game_screen->winner->name);
    SDL_AddTimer(5000, return_to_menu, this);
}

/**
 * @brief A fő programciklus.
 * 
 * @param this A Carcassone struktúra, ami tartalmazza az SDL kontextust.
 */
void Carcassone__run(Carcassone* this)
{
    this->is_running = true;

    // Ez egy másik projektemből van átmásolva.
    Uint64 now;
    Uint64 last = 0U;
    float accumulator = 0U;
    while(this->is_running) {
        // Calculate delta time
        now = SDL_GetTicks64();
        accumulator += MIN((float)(now - last), 1000.0f / FPS);
        last = now;
        for(float delta; accumulator >= (delta = 1.0f / FPS); accumulator -= delta) {
            SDL_LockMutex(this->smutex);
            switch(this->state) {
            case MENU:
                Carcassone__Menu__render(this);
                break;
            case LEADERBOARD:
                Carcassone__Lboard__render(this);
                break;
            case GAME:
                Carcassone__Game__render(this);
                break;
            }
            SDL_UnlockMutex(this->smutex);

            Carcassone__handle_input(this);
        }

    }
}
