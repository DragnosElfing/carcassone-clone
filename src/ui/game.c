#include <SDL2/SDL_events.h>
#include <stdlib.h>

#include "game/tile.h"
#include "utils.h"
#include "ui.h"
#include "app.h"

#ifdef _CRCLONE_DEBUG
    #include "debug/debugmalloc.h"
#endif

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
    this->game_screen->update_counter = true;
    this->game_screen->pile_counter = NULL;
    this->game_screen->drawn_tile = malloc(sizeof(Tile));
    this->game_screen->card_pile = CardPile__construct();
    for(size_t k = 0U; k < 4; ++k) {
        this->game_screen->held_arrow_keys[k] = 0;
    }
    for(size_t p = 0U; p < 2; ++p) {
        this->game_screen->players[p] = Player__construct(NULL, NULL, "", "");
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

    // nagyszerűen néz ki
    int max_width, height;
    TTF_SizeUTF8(this->small_font, "WWWWWWWWWWWWWWWWWWWWWWWW", &max_width, &height);

    SDL_Rect input1_max_rect = {
        this->width / 2 - max_width / 2, this->height / 4 + 150, max_width, height + 10
    };
    SDL_Rect input2_max_rect = {
        this->width / 2 - max_width / 2, this->height / 4 - 100, max_width, height + 10
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
    this->game_screen->end_turn_button =
        Carcassone__Button__construct(this, this->small_font, "KÖR VÉGE", (SDL_Rect){10, 10, 200, 60}, (SDL_Color){COLOR_WHITE}, 
            (SDL_Color){0, 0, 140, 255}, true);

    this->game_screen->active_input = &this->game_screen->player_name_inputs[1];

    this->game_screen->crown_texture = create_SDL_texture_from_BMP(this->renderer, "./res/winners_crown.bmp");

    SDL_StartTextInput();
}

/**
 * @brief Felszabadítja a megadott `Carcassone` struktúrához tartozó `GameScreen` által lefoglalt memóriát.
 *
 * @param this A `Carcassone` struktúra, aminek a lefoglalt memóriáját fel kell szabadítani.
 */
void Carcassone__Game__destroy(Carcassone* this)
{
    if(SDL_IsTextInputActive()) SDL_StopTextInput();
    
    destroy_SDL_Texture(this->game_screen->board_texture);
    destroy_SDL_Texture(this->game_screen->pile_counter);

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

void Carcassone__Game__handle_input(Carcassone* this, float dt)
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
        case SDL_KEYUP:
            if(!(SDL_SCANCODE_RIGHT <= event.key.keysym.scancode && event.key.keysym.scancode <= SDL_SCANCODE_UP)) return;

            unsigned int key = event.key.keysym.scancode - SDL_SCANCODE_RIGHT;
            this->game_screen->held_arrow_keys[key] = false;

            break;
        case SDL_KEYDOWN:
            switch(event.key.keysym.sym) {
                case SDLK_BACKSPACE:
                    if(get_utf8_length(this->game_screen->active_input->prompt.label) > 0) {
                        Carcassone__Prompt__edit(this, this->game_screen->active_input, NULL, false);
                    }
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
            if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->game_screen->player_name_inputs[0].prompt.global_rect)) {
                this->game_screen->active_input = &this->game_screen->player_name_inputs[0];
            } else if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->game_screen->player_name_inputs[1].prompt.global_rect)) {
                this->game_screen->active_input = &this->game_screen->player_name_inputs[1];
            }

            if(!this->game_screen->is_ready) {
                if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->game_screen->ready_button.global_rect)) {
                    if(!Carcassone__Game__check_names_valid(this)) break;
                    this->game_screen->is_ready = true;
                    Carcassone__Game__init_players(this);
                }
            } else {
                if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->game_screen->end_turn_button.global_rect)) {
                    if(this->game_screen->curr_player->has_placed_card) {
                        Carcassone__Game__calculate_points(this);
                        if(!Carcassone__Game__draw_new(this)) {
                            Carcassone__Game__wrapup(this);
                        }
                    }
                }
                if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->game_screen->concede_button.global_rect))
                    Carcassone__Game__wrapup(this);
                
                for(int y = 0; y < BOARD_SIZE; ++y) {
                    for(int x = 0; x < BOARD_SIZE; ++x) {
                        Tile* curr_tile = &this->game_screen->board[x][y];
                        if(!this->game_screen->curr_player->has_placed_card) {
                            if(Tile__point_in_tile(curr_tile, (SDL_FPoint){event.button.x, event.button.y})
                                    && curr_tile->type == EMPTY
                                    && Carcassone__Game__check_surrounding_tiles(this, (SDL_Point){x, y})
                            ) {
                                Tile__set_type(curr_tile, this->game_screen->drawn_tile->type, this->game_screen->drawn_tile->rotation);
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

    if(this->game_screen->is_ready) Carcassone__Game__move_board(this, dt);
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
            Carcassone__Game__indicate_possible_placements(this);
            Carcassone__Game__render_drawn_tile(this);
        }
        Carcassone__render_splash_title(this, &(SDL_Rect){this->game_screen->board_offset.x, 0, 400, 120});
        Carcassone__Game__render_player_stats(this);
        Carcassone__Button__render(this, &this->game_screen->concede_button);
        Carcassone__Button__render(this, &this->game_screen->end_turn_button);
        if(this->game_screen->is_game_over) {
            Carcassone__Game__render_game_over(this);
        }
    }
}

bool Carcassone__Game__check_names_valid(Carcassone* this)
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
void Carcassone__Game__init_players(Carcassone* this)
{
    for(size_t p = 0U; p < 2; ++p) {
        this->game_screen->players[p] = 
            Player__construct(this->renderer, this->small_font, this->game_screen->player_name_inputs[p].prompt.label, "./res/meeple_base.bmp");
    }

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
    // ? Lehetne ezt egy config fájlba tenni.
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
 * @brief A pakli tetején levő kártya és a pakli méretének renderelése.
 * 
 * @param this A Carcassone struktúra, ami tartalmazza az SDL kontextust.
 */
void Carcassone__Game__render_drawn_tile(Carcassone* this)
{
    SDL_Rect ts_rect = get_texture_rect_for(this->game_screen->drawn_tile->type);

    SDL_RenderCopyEx(this->renderer, this->game_screen->tileset_wrapper.tile_set, &ts_rect,
        &(SDL_Rect){this->game_screen->board_offset.x + 450, this->game_screen->board_offset.y - TILE_SIZE, TILE_SIZE-5, TILE_SIZE-5},
        this->game_screen->drawn_tile->rotation, NULL, SDL_FLIP_NONE);


    char counter_string[2+1];
    snprintf(counter_string, 3, "%zu", PILE_SIZE - this->game_screen->pile_index);
    int w, h;
    TTF_SizeText(this->default_font, counter_string, &w, &h);
    SDL_Rect pile_index_rect = {this->game_screen->board_offset.x + 555, this->game_screen->board_offset.y - TILE_SIZE/2.0f, w, h};
    if(this->game_screen->update_counter) {
        destroy_SDL_Texture(this->game_screen->pile_counter);

        SDL_Surface* curr_index_surface = TTF_RenderText_Blended(this->default_font, counter_string, (SDL_Color){COLOR_WHITE});
        if(curr_index_surface != NULL) {
            this->game_screen->pile_counter = SDL_CreateTextureFromSurface(this->renderer, curr_index_surface);
            SDL_FreeSurface(curr_index_surface);
        }

        this->game_screen->update_counter = false;
    }
    SDL_RenderCopy(this->renderer, this->game_screen->pile_counter, NULL, &pile_index_rect);

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

void Carcassone__Game__indicate_possible_placements(Carcassone* this)
{
    if(this->game_screen->curr_player->has_placed_card) return;
    
    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
            Tile* curr_tile = &this->game_screen->board[x][y];
            if(curr_tile->type != EMPTY) continue;

            if(Carcassone__Game__check_surrounding_tiles(this, (SDL_Point){x, y})) {
                SDL_SetRenderDrawColor(this->renderer, 20, 240, 100, 255);
                SDL_RenderFillRect(this->renderer, 
                &(SDL_Rect){
                        curr_tile->global_coords.x, curr_tile->global_coords.y, TILE_SIZE, TILE_SIZE}
                );
            }
        }
    }
}

bool Carcassone__Game__check_if_possible(Carcassone* this)
{
    if(this->game_screen->drawn_tile == NULL) return false;

    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
            Tile* curr_tile = &this->game_screen->board[x][y];
            if(curr_tile->type != EMPTY) continue;

            for(unsigned int rot = 0U; rot < 4; ++rot) {
                if(Carcassone__Game__check_surrounding_tiles(this, (SDL_Point){x, y})) {
                    return true;
                }

                if(!this->game_screen->drawn_tile->rotatable) break;
                else Tile__rotate(this->game_screen->drawn_tile);
            }

        }
    }

    return false;
}

void Carcassone__Game__calculate_points(Carcassone* this)
{
    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
            Tile* curr_tile = &this->game_screen->board[x][y];
            if(curr_tile->type == EMPTY || curr_tile->is_scored) continue;

            Carcassone__Game__calculate_scores_for_graph(this, curr_tile, ROAD);
            Carcassone__Game__calculate_scores_for_graph(this, curr_tile, CASTLE);
            Carcassone__Game__calculate_scores_for_cloister(this, curr_tile);
        }
    }
}

void Carcassone__Game__calculate_scores_for_cloister(Carcassone* this, Tile* tile)
{
    if(this->game_screen->is_game_over) return;
    
    if(tile->type == FIELD_CLOISTER_ROAD_S || tile->type == FIELD_CLOISTER_ROAD_NS) {
        tile->is_expired = true;
        for(int yrel = tile->board_coords.y-1; yrel <= tile->board_coords.y+1; ++yrel) {
            for(int xrel = tile->board_coords.x-1; xrel <= tile->board_coords.x+1; ++xrel) {
                if(yrel < 0 || yrel >= BOARD_SIZE || xrel < 0 || xrel >= BOARD_SIZE) continue;
                if(this->game_screen->board[xrel][yrel].type == EMPTY) {
                    return;
                }
            }
        }
    } else return;

    for(size_t p = 0U; p < 2; ++p) {
        for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
            Meeple* curr_m = &this->game_screen->players[p].meeples[m];
            if(!curr_m->is_placed) continue;
            if(curr_m->x != tile->board_coords.x || curr_m->y != tile->board_coords.y) continue;

            Player__add_to_score(&this->game_screen->players[p], 3);
            Player__reclaim_meeple(&this->game_screen->players[p], curr_m);
            tile->is_scored = true;
            DBG_LOG("Added score (cloister) to: %s", this->game_screen->players[p].name);
        }
    }
}

// TODO
void Carcassone__Game__calculate_scores_for_graph(Carcassone* this, Tile* tile, ConnectionType conn_type)
{
    bool has_approp_conn = false;
    for(unsigned int dir = NORTH; dir <= WEST; ++dir) {
        if(tile->connections[dir] == conn_type) {
            has_approp_conn = true;
            break;
        }
    }
    if(!has_approp_conn) return;

    unsigned int points = 0U;

    SDL_Point dir_rel_coords[4] = {
        {0, -1}, {1, 0}, {0, 1}, {-1, 0}
    };


    size_t visited_idx = 0U;
    Tile* visited[PILE_SIZE] = {0};
    size_t stack_idx = 0U;
    Tile* stack[PILE_SIZE] = {0};
        
    stack[stack_idx] = tile;
    ++stack_idx;
    while(stack_idx != 0U) {
        --stack_idx;
        Tile* popped = stack[stack_idx];
        if(!this->game_screen->is_game_over && popped->type == EMPTY) return;
        
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
            if(popped->type != EMPTY) ++points;

            switch(conn_type) {
                case CASTLE:
                    if(8 <= popped->type && popped->type <= 9) ++points;
                case ROAD:
                    if(0 <= popped->type && popped->type <= 3) ++points;
                default: break;
            }

            visited[visited_idx] = popped;
            ++visited_idx;
        }

        for(ConnectionDirection dir = NORTH; dir <= WEST; ++dir) {
            int new_x = popped->board_coords.x + dir_rel_coords[dir].x;
            int new_y = popped->board_coords.y + dir_rel_coords[dir].y;
            if(new_x < 0 || new_x >= BOARD_SIZE || new_y < 0 || new_y >= BOARD_SIZE) continue;

            if(popped->connections[dir] == conn_type) {
                stack[stack_idx] = &this->game_screen->board[new_x][new_y];
                ++stack_idx;
            }
        }
    }

    for(size_t i = 0U; i < visited_idx; ++i) {
        for(size_t p = 0U; p < 2; ++p) {
            for(size_t m = 0U; m < MAX_MEEPLES; ++m) {
                Meeple* curr_m = &this->game_screen->players[p].meeples[m];
                if(!curr_m->is_placed) continue;
                if(curr_m->x != visited[i]->board_coords.x || curr_m->y != visited[i]->board_coords.y) continue;

                Player__add_to_score(&this->game_screen->players[p], points);
                Player__reclaim_meeple(&this->game_screen->players[p], curr_m);
                visited[i]->is_scored = true;
                DBG_LOG("Added score (%u) to: %s", conn_type, this->game_screen->players[p].name);
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
void Carcassone__Game__move_board(Carcassone* this, float dt)
{
    float mvx = 0.0f;
    float mvy = 0.0f;
    float const speed = 0.5f;

    if(this->game_screen->held_arrow_keys[0]) {
        mvx -= speed * dt;
    }
    if(this->game_screen->held_arrow_keys[1]) {
        mvx += speed * dt;
    }
    if(this->game_screen->held_arrow_keys[2]) {
        mvy -= speed * dt;
    }
    if(this->game_screen->held_arrow_keys[3]) {
        mvy += speed * dt;
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
bool Carcassone__Game__check_surrounding_tiles(Carcassone* this, SDL_Point tcoords)
{
    int x = tcoords.x;
    int y = tcoords.y;

    SDL_Point dir_rel_coords[4] = {
        {0, -1}, {1, 0}, {0, 1}, {-1, 0}
    };

    bool next_to_placed = false;
    for(ConnectionDirection dir = NORTH; dir <= WEST; ++dir) {
        int rel_x = x + dir_rel_coords[dir].x;
        int rel_y = y + dir_rel_coords[dir].y;
        if(rel_y < 0 || rel_y >= BOARD_SIZE || rel_x < 0 || rel_x >= BOARD_SIZE) continue;
        
        ConnectionDirection neighbour_dir = (dir + 2) % 4;
        if(this->game_screen->board[rel_x][rel_y].type != EMPTY) {
            next_to_placed = true;
        }
        if(this->game_screen->board[rel_x][rel_y].connections[neighbour_dir] != NONE && 
            this->game_screen->board[rel_x][rel_y].connections[neighbour_dir] != this->game_screen->drawn_tile->connections[dir]) {
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
bool Carcassone__Game__draw_new(Carcassone* this)
{
    TileType next_type;
    this->game_screen->card_pile = CardPile__pop(this->game_screen->card_pile, &next_type);
    if(this->game_screen->card_pile != NULL) {
        Tile__construct(this->game_screen->drawn_tile, next_type, (SDL_Point){-1, -1}, (SDL_FPoint){0, 0});
        ++this->game_screen->pile_index;
        this->game_screen->update_counter = true;

        // khm.
        if(this->game_screen->curr_player->name == this->game_screen->players[0].name) {
            this->game_screen->curr_player = &this->game_screen->players[1];
        } else {
            this->game_screen->curr_player = &this->game_screen->players[0];
        }

        if(this->game_screen->curr_player != NULL) {
            this->game_screen->curr_player->has_placed_card = false;
        }
    }

    if(this->game_screen->pile_index >= PILE_SIZE) {
        return false;
    } else if(!this->game_screen->curr_player->has_placed_card && !Carcassone__Game__check_if_possible(this)) {
        Carcassone__Game__draw_new(this);
    }

    return true;
}

static unsigned int return_to_menu(unsigned int interval, void* this)
{  
    Carcassone* thiz = (Carcassone*) this;
    SDL_LockMutex(thiz->smutex);
    Carcassone__switch_state(thiz, MENU);
    SDL_UnlockMutex(thiz->smutex);
    return 0;
}
void Carcassone__Game__wrapup(Carcassone* this)
{
    this->game_screen->is_game_over = true;

    if(this->game_screen->players[0].score == this->game_screen->players[1].score) {
        this->game_screen->winner = NULL;
    } else {
        this->game_screen->winner = 
            this->game_screen->players[0].score > this->game_screen->players[1].score ? &this->game_screen->players[0] : &this->game_screen->players[1];
    }
    if(this->lboard_screen->leaderboard != NULL) {
        Leaderboard__insert_new(this->lboard_screen->leaderboard, this->game_screen->winner);
        Leaderboard__load(this->lboard_screen->leaderboard);
    }

    SDL_AddTimer(5000, return_to_menu, this);
}