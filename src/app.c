#include <SDL2/SDL_scancode.h>
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
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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

    new_app->default_font = TTF_OpenFont("res/fonts/Sedan_SC/sedan_sc.ttf", 128);
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

    return new_app;
}

/**
 * @brief Felszabadítja a megadott Carcassone struktúra által lefoglalt memóriát.
 *
 * @param this A Carcassone struktúra, aminek a lefoglalt memóriáját fel kell szabadítani.
 */
void Carcassone__destroy(Carcassone* this)
{
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
            break;
        case GAME:
            this->game_screen->is_ready = false;
            Carcassone__init_board(this);
            Carcassone__init_pile(this);
            break;
        case LEADERBOARD:
            // TODO: reload records
            break;
    }
}


static void remove_last_utf8_char(char *str) {
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
        p--;
    }
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

            if(this->game_screen->player_name_inputs[0].is_active) {
                if(strlen(this->game_screen->player_name_inputs[0].prompt.label) < 24) {
                    Carcassone__Prompt__edit(this, &this->game_screen->player_name_inputs[0], event.text.text, true);
                }
            } else if(this->game_screen->player_name_inputs[1].is_active){
                if(strlen(this->game_screen->player_name_inputs[1].prompt.label) < 24) {
                    Carcassone__Prompt__edit(this, &this->game_screen->player_name_inputs[1], event.text.text, true);
                }
            }
            break;
        case SDL_TEXTEDITING:
            DBG_LOG("TEXT EDITING EVENT");
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
                    // TODO: ABSOLUTELY NOT, and handle utf8 characters
                    if(this->game_screen->player_name_inputs[0].is_active) {
                        if(strlen(this->game_screen->player_name_inputs[0].prompt.label) > 0) {
                            remove_last_utf8_char(this->game_screen->player_name_inputs[0].prompt.label);
                            Carcassone__Prompt__edit(this, &this->game_screen->player_name_inputs[0], this->game_screen->player_name_inputs[0].prompt.label, false);
                        }
                    } else if(this->game_screen->player_name_inputs[1].is_active){
                        if(strlen(this->game_screen->player_name_inputs[1].prompt.label) > 0) {
                            remove_last_utf8_char(this->game_screen->player_name_inputs[1].prompt.label);
                            Carcassone__Prompt__edit(this, &this->game_screen->player_name_inputs[1], this->game_screen->player_name_inputs[1].prompt.label, false);
                        }
                    }
                    break;
                case SDLK_d:
                    if(this->game_screen->is_ready) Carcassone__draw_new(this);
                    break;
                case SDLK_r:
                    if(this->game_screen->is_ready) Tile__rotate(this->game_screen->drawn_tile);
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
                if(this->game_screen->player_name_inputs[0].is_active) break;
                Carcassone__Prompt__toggle_focus(this, &this->game_screen->player_name_inputs[0]);
                Carcassone__Prompt__toggle_focus(this, &this->game_screen->player_name_inputs[1]);
            } else if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->game_screen->player_name_inputs[1].prompt.global_rect)) {
                if(this->game_screen->player_name_inputs[1].is_active) break;
                Carcassone__Prompt__toggle_focus(this, &this->game_screen->player_name_inputs[1]);
                Carcassone__Prompt__toggle_focus(this, &this->game_screen->player_name_inputs[0]);
            }

            if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->game_screen->ready_button.global_rect)) {
                this->game_screen->is_ready = true;
                Carcassone__init_players(this);
            }

            for(int y = 0; y < BOARD_SIZE; ++y) {
                for(int x = 0; x < BOARD_SIZE; ++x) {
                    Tile* curr_tile = &this->game_screen->board[x][y];
                    if(Tile__point_in_tile(curr_tile, (SDL_Point){event.button.x, event.button.y})
                            && curr_tile->type == EMPTY
                            && Carcassone__check_surrounding_tiles(this, (SDL_Point){x, y})
                    ) {
                        // TODO
                        SDL_Point temp_new_local = {curr_tile->local_coords.x / TILE_SIZE,
                            curr_tile->local_coords.y / TILE_SIZE};
                        Tile__construct(curr_tile, this->game_screen->drawn_tile->type,
                            temp_new_local, this->game_screen->board_offset);
                        Tile__set_rotation(curr_tile, this->game_screen->drawn_tile->rotation);

                        Carcassone__draw_new(this);
                    }
                }
            }
            break;
        default:
            break;
    }

    if(this->game_screen->is_ready) Carcassone__move_board(this, 0U);
}

/**
 * @brief Játékosok létrehozása.
 * 
 * Inicializálja a két játékost előre megadott adatok alapján (ezek a GameScreenben találhatók).
 *
 * @param this A Carcassone struktúra.
 */
void Carcassone__init_players(Carcassone* this) // TODO
{
    this->game_screen->players[0] = Player__construct(this->renderer, this->default_font,
            this->game_screen->player_name_inputs[0].prompt.label, 
            Leaderboard__get_highscore_for(this->lboard_screen->leaderboard, this->game_screen->player_name_inputs[0].prompt.label)); // TODO: NO
    this->game_screen->players[1] = Player__construct(this->renderer, this->default_font,
            this->game_screen->player_name_inputs[1].prompt.label, 
            Leaderboard__get_highscore_for(this->lboard_screen->leaderboard, this->game_screen->player_name_inputs[1].prompt.label)); // TODO: NO

    Player__toggle_turn_active(&this->game_screen->players[0]);
}

/**
 * @brief Kártyapakli létrehozása.
 * 
 * Véletlenszerűen megkeveri a paklit és létrehozza a mezőkártyákat mindegyikhez.
 *
 * @param this A Carcassone struktúra.
 */
void Carcassone__init_pile(Carcassone* this)
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
        this->game_screen->card_pile[n] = malloc(sizeof(Tile));
        Tile__construct(this->game_screen->card_pile[n], pile[n], (SDL_Point){0, 0}, (SDL_Point){0, 0});
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
void Carcassone__init_board(Carcassone* this)
{
    // BOARD_SIZE * BOARD_SIZE 2D array
    this->game_screen->board = malloc(BOARD_SIZE * sizeof(Tile*));
    for(size_t n = 0U; n < BOARD_SIZE; ++n) {
        this->game_screen->board[n] = malloc(BOARD_SIZE * sizeof(Tile));
    }

    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
            Tile__construct(&this->game_screen->board[x][y], EMPTY, (SDL_Point){x, y}, this->game_screen->board_offset);
        }
    }

    // Kezdőkártya
    Tile__construct(&this->game_screen->board[BOARD_SIZE / 2][BOARD_SIZE / 2], CASTLE_CAP_WALL_ROAD_BY,
        (SDL_Point){BOARD_SIZE / 2, BOARD_SIZE / 2}, this->game_screen->board_offset);
}

/**
 * @brief Játéktábla renderelése.
 * 
 * @param this A Carcassone struktúra, ami tartalmazza az SDL kontextust.
 */
void Carcassone__render_board(Carcassone* this)
{
    SDL_Rect viewport_rect = {this->game_screen->board_offset.x, this->game_screen->board_offset.y, 600, this->height - this->game_screen->board_offset.y - 10};
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

            SDL_Rect ts_rect = TilesetWrapper__get_texture_rect_for(&this->game_screen->tileset_wrapper, curr_tile->type);
            SDL_RenderCopyEx(this->renderer, this->game_screen->tileset_wrapper.tile_set, &ts_rect, &tile_rect,
                curr_tile->rotation, NULL, SDL_FLIP_NONE);

            // Fehér keret a celláknak
            SDL_SetRenderDrawColor(this->renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(this->renderer, &tile_rect);
        }
    }

    SDL_SetRenderTarget(this->renderer, NULL);
    SDL_RenderCopy(this->renderer, this->game_screen->board_texture, NULL, &viewport_rect);
    //SDL_RenderDrawRect(this->renderer, &viewport_rect);
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
void Carcassone__render_drawn_tile(Carcassone* this)
{
    SDL_Rect ts_rect = TilesetWrapper__get_texture_rect_for(&this->game_screen->tileset_wrapper, this->game_screen->drawn_tile->type);

    SDL_RenderCopyEx(this->renderer, this->game_screen->tileset_wrapper.tile_set, &ts_rect,
        &(SDL_Rect){this->game_screen->board_offset.x + 450, this->game_screen->board_offset.y - TILE_SIZE, TILE_SIZE-5, TILE_SIZE-5},
        this->game_screen->drawn_tile->rotation, NULL, SDL_FLIP_NONE);

    SDL_Rect pile_index_rect = {this->game_screen->board_offset.x + 555, this->game_screen->board_offset.y - TILE_SIZE/2, 50, 50};
    if(PILE_SIZE - this->game_screen->pile_index < 10) {
        pile_index_rect.w /= 2;
        pile_index_rect.x += pile_index_rect.w/2;
    }
    SDL_RenderCopy(this->renderer, this->game_screen->pile_counter[this->game_screen->pile_index], NULL, &pile_index_rect);

    // TODO: temp
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    SDL_RenderCopyEx(this->renderer, this->game_screen->tileset_wrapper.tile_set, &ts_rect,
        &(SDL_Rect){mx - TILE_SIZE / 2, my - TILE_SIZE / 2, TILE_SIZE, TILE_SIZE},
        this->game_screen->drawn_tile->rotation, NULL, SDL_FLIP_NONE);
}

/**
 * @brief A splash cím renderelése.
 * 
 * @param this A Carcassone struktúra, ami tartalmazza az SDL kontextust.
 */
void Carcassone__render_player_stats(Carcassone* this) // TODO
{   
    if(this->game_screen->players[0].stat_panel != NULL) {
        SDL_RenderCopy(this->renderer, this->game_screen->players[0].stat_panel, NULL, &(SDL_Rect){0, 0, 300, 700});
    }

    if(this->game_screen->players[1].stat_panel != NULL) {
        SDL_RenderCopy(this->renderer, this->game_screen->players[1].stat_panel, NULL, 
            &(SDL_Rect){this->width - 300, 0, 300, 700});
    }

    SDL_SetRenderDrawColor(this->renderer, 255, 0, 0, 255);
    if(this->game_screen->players[0].is_turn_active) {
        SDL_RenderDrawRect(this->renderer, &(SDL_Rect){0, 0, 300, 700});
    } else if(this->game_screen->players[1].is_turn_active) {
        SDL_RenderDrawRect(this->renderer, &(SDL_Rect){this->width - 300, 0, 300, 700});
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
void Carcassone__move_board(Carcassone* this, SDL_Scancode key) // TODO
{
    float mvx = 0;
    float mvy = 0;

    // TODO: global state, check KEYDOWN and KEYUP
    if(this->game_screen->held_arrow_keys[0]) {
        mvx -= 0.01f;
    }
    if(this->game_screen->held_arrow_keys[1]) {
        mvx += 0.01f;
    }
    if(this->game_screen->held_arrow_keys[2]) {
        mvy -= 0.01f;
    }
    if(this->game_screen->held_arrow_keys[3]) {
        mvy += 0.01f;
    }

    for(int y = 0U; y < BOARD_SIZE; ++y) {
        for(int x = 0U; x < BOARD_SIZE; ++x) {
            Tile__move_by(&this->game_screen->board[x][y], mvx, mvy);
        }
    }
}

/**
 * @brief A splash cím renderelése.
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
void Carcassone__draw_new(Carcassone* this) // TODO
{
    this->game_screen->drawn_tile = this->game_screen->card_pile[this->game_screen->pile_index];
    ++this->game_screen->pile_index;
    for(size_t p = 0U; p < 2; ++p) {
        Player__toggle_turn_active(&this->game_screen->players[p]);
    }

    if(this->game_screen->pile_index >= PILE_SIZE) {
        DBG_LOG("Kifogytunk a kártyákból!");
        Carcassone__switch_state(this, MENU);
    }
}

/**
 * @brief A fő programciklus.
 * 
 * @param this A Carcassone struktúra, ami tartalmazza az SDL kontextust.
 */
void Carcassone__run(Carcassone* this)
{
    this->is_running = true;
    // Uint64 now;
    // Uint64 last = 0U;
    // float accumulator = 0U;
    // while(this->is_running) {
    //     // Calculate delta time
    //     now = SDL_GetTicks64();
    //     accumulator += MIN((float)(now - last), 1000.0f / FPS);
    //     last = now;
    //     for(float delta; accumulator >= (delta = 1.0f / FPS); accumulator -= delta) { // TOOD: this is just bad
    //         printf("ham!\n");
    //         switch(this->state) {
    //         case MENU:
    //             Carcassone__Menu__render(this);
    //             break;
    //         case LEADERBOARD:
    //             Carcassone__Lboard__render(this);
    //             break;
    //         case GAME:
    //             Carcassone__Game__render(this);
    //             break;
    //         }
    //     }

    //     Carcassone__handle_input(this);
    // }

    while(this->is_running) {
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

        Carcassone__handle_input(this);
    }

}
