#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _CRCLONE_DEBUG
#include "debug/debugmalloc.h"
#endif

#include "app.h"
#include "tile.h"

Carcassone* Carcassone__construct(int width, int height, char const* title)
{
    srand(time(NULL));

    // Hogy megjelenjenek a RENDER kategóriás hibák is (és minden egyéb)
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

    Carcassone* new_app = malloc(sizeof(Carcassone));
    // Default értékek
    new_app->width = width;
    new_app->height = height;
    new_app->is_running = false;
    new_app->map_offset = (SDL_Point){width/2-300, 120};
    new_app->board = NULL;
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
    //return NULL; // ? why segfault
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

    new_app->default_font = TTF_OpenFont("res/fonts/Sedan_SC/sedan_sc.ttf", 96);
    if(new_app->default_font == NULL) {
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

    Carcassone__Menu__construct(new_app);
    // Default értékek
    Carcassone__init_players(new_app);
    new_app->tileset_wrapper = TilesetWrapper__construct(new_app->renderer);
    if(new_app->tileset_wrapper.tile_set == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Nem lehetett betölteni az atlaszt!");
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "%s", SDL_GetError());
    }
    Carcassone__init_board(new_app);
    new_app->pile_index = 0U;
    Carcassone__init_pile(new_app);
    Carcassone__draw_new(new_app);

    return new_app;
}

void Carcassone__Menu__construct(Carcassone* this)
{
    this->menu = malloc(sizeof(Menu));
    this->menu->background = SDL_CreateTextureFromSurface(this->renderer, SDL_LoadBMP("./res/menu_bg.bmp"));
    if(this->menu->background == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Nem lehetett betölteni az háttérképet!");
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "%s", SDL_GetError());
    } else {
        //SDL_SetTextureBlendMode(this->menu->background, SDL_BLENDMODE_MUL);
    }
    SDL_SetRenderDrawBlendMode(this->renderer, SDL_BLENDMODE_BLEND);

    // TODO: rel coords, viewport
    this->menu->button_container = (SDL_FRect){this->width/2.0f - 400, 250, 800, 600};

    // TODO: seperate Button__construct
    this->menu->start_button = (Button){
        .rect = {
            this->menu->button_container.x + this->menu->button_container.w / 2 - 200 - 100,
            this->menu->button_container.y + this->menu->button_container.h / 2 - 60, 
            200, 120
        },
        .label = "Start"
    };
    this->menu->start_button.label_texture = SDL_CreateTextureFromSurface(
        this->renderer, TTF_RenderUTF8_Blended(this->default_font, this->menu->start_button.label, 
        (SDL_Color){255, 255, 255, 255}));
    this->menu->lboard_button = (Button){
        .rect = {
            this->menu->button_container.x + this->menu->button_container.w / 2 + 200 - 100,
            this->menu->button_container.y + this->menu->button_container.h / 2 - 60, 
            200, 120
        },
        .label = "Dicsőséglista"
    };
    this->menu->lboard_button.label_texture = SDL_CreateTextureFromSurface(
        this->renderer, TTF_RenderUTF8_Blended(this->default_font, this->menu->lboard_button.label, 
        (SDL_Color){255, 255, 255, 255}));
    
}
void Carcassone__Menu__destroy(Carcassone* this)
{
    if(this->menu->background != NULL) SDL_DestroyTexture(this->menu->background);
    free(this->menu);
}

void Carcassone__destroy(Carcassone* this)
{
    this->is_running = false;

    Carcassone__Menu__destroy(this);
    // Először az SDL áll le
    // TODO: ? no
    if(this->window_icon != NULL)   SDL_FreeSurface(this->window_icon);
    if(this->splash_title != NULL)  SDL_DestroyTexture(this->splash_title);
    if(this->window != NULL)        SDL_DestroyWindow(this->window);
    if(this->renderer != NULL)      SDL_DestroyRenderer(this->renderer);
    if(this->default_font != NULL)  TTF_CloseFont(this->default_font);
    if(TTF_WasInit() != 0)          TTF_Quit();
    if(SDL_WasInit(0) != 0)         SDL_Quit();

    // Utánna minden mást
    if(this->board != NULL) {
        for(size_t n = 0U; n < GRID_SIZE; ++n) {
            free(this->board[n]);
        }
        free(this->board);
    }
    for(size_t n = 0U; n < PILE_SIZE; ++n) {
        if(this->card_pile[n] != NULL) {
            free(this->card_pile[n]);
        }
    }
    TilesetWrapper__destroy(&this->tileset_wrapper);
    free(this);
}

void Carcassone__handle_input(Carcassone* this)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch(event.type) {
        case SDL_QUIT:
            this->is_running = false;
            break;
        case SDL_KEYDOWN:
            if(this->state != GAME) break;

            switch(event.key.keysym.sym) {
                case SDLK_d:
                    Carcassone__draw_new(this);
                    break;
                case SDLK_r:
                    Tile__rotate(this->drawn_tile);
                    break;
                default:
                    Carcassone__move_board(this, event.key.keysym.sym);
                    break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->menu->start_button.rect)) {
                this->state = GAME;
            }
            if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->menu->lboard_button.rect)) {
                this->state = LBOARD;

            }

            if(this->state != GAME) break;

            for(int y = 0; y < GRID_SIZE; ++y) {
                for(int x = 0; x < GRID_SIZE; ++x) {
                    Tile* curr_tile = &this->board[x][y];
                    if(Tile__point_in_tile(curr_tile, (SDL_Point){event.button.x, event.button.y})
                            && curr_tile->type == EMPTY
                            && Carcassone__check_surrounding_tiles(this, (SDL_Point){x, y})
                    ) {
                        // TODO
                        SDL_Point temp_new_local = {curr_tile->local_coords.x / TILE_SIZE, curr_tile->local_coords.y / TILE_SIZE};
                        Tile__construct(curr_tile, this->drawn_tile->type,
                            temp_new_local, this->map_offset);
                        Tile__set_rotation(curr_tile, this->drawn_tile->rotation);

                        Carcassone__draw_new(this);
                    }
                }
            }
            break;
        default:
            break;
    }
}

void Carcassone__render(Carcassone* this)
{
    SDL_SetRenderDrawColor(this->renderer, 102, 102, 153, 255);
    SDL_RenderClear(this->renderer);

    Carcassone__render_board(this);
    Carcassone__render_splash_title(this);
    //Carcassone__render_player_stats(this);
    Carcassone__render_drawn_tile(this);

    SDL_RenderPresent(this->renderer);
}

void Carcassone__init_players(Carcassone* this)
{
    this->players[0] = (Player){.name = "Játékos Egy", .score = 0U, .is_turn_over = true};
    this->players[1] = (Player){.name = "Játékos Kettő", .score = 0U, .is_turn_over = true};
}

void Carcassone__init_pile(Carcassone* this)
{
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
        size_t rand_index = rand() % PILE_SIZE;
        TileType temp = pile[n];

        pile[n] = pile[rand_index];
        pile[rand_index] = temp;
    }

    for(size_t n = 0U; n < PILE_SIZE; ++n) {
        this->card_pile[n] = malloc(sizeof(Tile));
        Tile__construct(this->card_pile[n], pile[n], (SDL_Point){0, 0}, (SDL_Point){0, 0});
    }

}

void Carcassone__init_board(Carcassone* this)
{
    // GRID_SIZE * GRID_SIZE 2D array
    this->board = malloc(GRID_SIZE * sizeof(Tile*));
    for(size_t n = 0U; n < GRID_SIZE; ++n) {
        this->board[n] = malloc(GRID_SIZE * sizeof(Tile));
    }

    for(int y = 0; y < GRID_SIZE; ++y) {
        for(int x = 0; x < GRID_SIZE; ++x) {
            Tile__construct(&this->board[x][y], EMPTY, (SDL_Point){x, y}, this->map_offset);
        }
    }

    // Kezdőkártya
    Tile__construct(&this->board[GRID_SIZE / 2][GRID_SIZE / 2], CASTLE_CAP_WALL_ROAD_BY,
        (SDL_Point){GRID_SIZE / 2, GRID_SIZE / 2}, this->map_offset);
}

void Carcassone__render_board(Carcassone* this)
{
    SDL_Rect viewport_rect = {this->map_offset.x, this->map_offset.y, 600, this->height - this->map_offset.y - 10};
    SDL_RenderSetViewport(this->renderer, &viewport_rect);

    // Cellák felrajzolása egy külön textúrára
    for(int y = 0U; y < GRID_SIZE; ++y) {
        for(int x = 0U; x < GRID_SIZE; ++x) {
            Tile* curr_tile = &this->board[x][y];
            SDL_Rect tile_rect = {
                curr_tile->local_coords.x,
                curr_tile->local_coords.y,
                TILE_SIZE, TILE_SIZE
            };

            SDL_Rect ts_rect = TilesetWrapper__get_texture_rect_for(&this->tileset_wrapper, curr_tile->type);
            SDL_RenderCopyEx(this->renderer, this->tileset_wrapper.tile_set, &ts_rect, &tile_rect,
                curr_tile->rotation, NULL, SDL_FLIP_NONE);

            // Fehér keret a celláknak
            SDL_SetRenderDrawColor(this->renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(this->renderer, &tile_rect);
        }
    }
    SDL_RenderSetViewport(this->renderer, NULL);
    SDL_RenderDrawRect(this->renderer, &viewport_rect);
}

void Carcassone__render_splash_title(Carcassone* this)
{
    // TODO: too many arbitrary values
    SDL_RenderCopy(this->renderer, this->splash_title, NULL,
        &(SDL_Rect){this->map_offset.x, 0, 400, 120});
}

void Carcassone__render_drawn_tile(Carcassone* this)
{
    SDL_Rect ts_rect = TilesetWrapper__get_texture_rect_for(&this->tileset_wrapper, this->drawn_tile->type);
    // if(tile_texture == NULL) {
    //     SDL_LogCritical(SDL_LOG_CATEGORY_RENDER, "Nem sikerült megtalálni a megfelelő textúrát!");
    //     SDL_LogCritical(SDL_LOG_CATEGORY_RENDER, "%s", SDL_GetError());
    // }

    SDL_RenderCopyEx(this->renderer, this->tileset_wrapper.tile_set, &ts_rect,
        &(SDL_Rect){this->map_offset.x + 450, this->map_offset.y - TILE_SIZE, TILE_SIZE-5, TILE_SIZE-5},
        this->drawn_tile->rotation, NULL, SDL_FLIP_NONE);

    char counter_string[2];
    sprintf(counter_string, "%zu", PILE_SIZE - this->pile_index);

    // SDL_Texture* pile_counter = SDL_CreateTextureFromSurface(this->renderer,
    //     TTF_RenderText_Shaded(this->default_font, counter_string,
    //         (SDL_Color){255, 255, 255, 255}, (SDL_Color){102, 102, 153, 255})); // TODO: transparent, change pixelformat
    // if(pile_counter != NULL) {
    //     SDL_Rect text_rect = {this->map_offset.x + 555, this->map_offset.y - TILE_SIZE/2, 50, 50};
    //     if(PILE_SIZE - this->pile_index < 10) {
    //         text_rect.w /= 2;
    //         text_rect.x += text_rect.w/2;
    //     }
    //     SDL_RenderCopy(this->renderer, pile_counter, NULL, &text_rect);
    //     SDL_DestroyTexture(pile_counter);
    // }

    // TODO: temp
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    SDL_RenderCopyEx(this->renderer, this->tileset_wrapper.tile_set, &ts_rect,
        &(SDL_Rect){mx - TILE_SIZE / 2, my - TILE_SIZE / 2, TILE_SIZE, TILE_SIZE},
        this->drawn_tile->rotation, NULL, SDL_FLIP_NONE);
}

void Carcassone__render_player_stats(Carcassone* this)
{
    SDL_Texture* player1_handle = SDL_CreateTextureFromSurface(this->renderer,
        TTF_RenderUTF8_Shaded(this->default_font, this->players[0].name,
            (SDL_Color){100, 190, 255, 255}, (SDL_Color){102, 102, 153, 255}));
    if(player1_handle != NULL) {
        SDL_RenderCopy(this->renderer, player1_handle, NULL, &(SDL_Rect){10, 10, 200, 50});
        SDL_DestroyTexture(player1_handle);
    }

    SDL_Texture* player2_handle = SDL_CreateTextureFromSurface(this->renderer,
        TTF_RenderUTF8_Shaded(this->default_font, this->players[1].name,
            (SDL_Color){255, 190, 100, 255}, (SDL_Color){102, 102, 153, 255}));
    if(player2_handle != NULL) {
        SDL_RenderCopy(this->renderer, player2_handle, NULL,
            &(SDL_Rect){this->map_offset.x + 650, 10, 200, 50});
        SDL_DestroyTexture(player2_handle);
    }
}

void Carcassone__move_board(Carcassone* this, SDL_KeyCode key)
{
    float mvx = 0.0f;
    float mvy = 0.0f;

    // TODO: global state, check KEYDOWN and KEYUP
    if(key == SDLK_UP)
        mvy += 1;
    if(key == SDLK_DOWN)
        mvy += -1;
    if(key == SDLK_LEFT)
        mvx += 1;
    if(key == SDLK_RIGHT)
        mvx += -1;

    for(int y = 0U; y < GRID_SIZE; ++y) {
        for(int x = 0U; x < GRID_SIZE; ++x) {
            Tile__move_by(&this->board[x][y], mvx, mvy);
        }
    }
}

bool Carcassone__check_surrounding_tiles(Carcassone* this, SDL_Point tcoords)
{
    int x = tcoords.x;
    int y = tcoords.y;

    // TODO: for loop
    bool next_to_placed = false;
    if(y - 1 >= 0) {
        if(this->board[x][y-1].type != EMPTY) next_to_placed = true;
        if(this->board[x][y-1].south != this->drawn_tile->north && this->board[x][y-1].south != NONE) {
            return false;
        }
    }
    if(y + 1 < GRID_SIZE) {
        if(this->board[x][y+1].type != EMPTY) next_to_placed = true;
        if(this->board[x][y+1].north != this->drawn_tile->south && this->board[x][y+1].north != NONE) {
            return false;
        }
    }
    if(x - 1 >= 0) {
        if(this->board[x-1][y].type != EMPTY) next_to_placed = true;
        if(this->board[x-1][y].east != this->drawn_tile->west && this->board[x-1][y].east != NONE) {
            return false;
        }
    }
    if(x + 1 < GRID_SIZE) {
        if(this->board[x+1][y].type != EMPTY) next_to_placed = true;
        if(this->board[x+1][y].west != this->drawn_tile->east && this->board[x+1][y].west != NONE) {
            return false;
        }
    }

    return next_to_placed;

}

void Carcassone__draw_new(Carcassone* this)
{
    this->drawn_tile = this->card_pile[this->pile_index];
    ++this->pile_index;
    if(this->pile_index >= PILE_SIZE ) {
        SDL_Log("Kifogytunk a kártyákból!");
        this->is_running = false;
    }
}

void Carcassone__Menu__render(Carcassone* this)
{
    SDL_RenderClear(this->renderer);
    SDL_RenderCopy(this->renderer, this->menu->background, NULL, NULL);

    SDL_SetRenderDrawColor(this->renderer, 255, 165, 105, 155);
    SDL_RenderFillRectF(this->renderer, NULL);

    SDL_SetRenderDrawColor(this->renderer, 255, 255, 255, 200);
    SDL_RenderFillRectF(this->renderer, &this->menu->button_container);
    SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255);
    SDL_RenderDrawRectF(this->renderer, &this->menu->button_container);

    SDL_RenderFillRect(this->renderer, &this->menu->start_button.rect);
    SDL_RenderCopy(this->renderer, 
        this->menu->start_button.label_texture, NULL, &this->menu->start_button.rect);
    SDL_RenderFillRect(this->renderer, &this->menu->lboard_button.rect);
    SDL_RenderCopy(this->renderer, 
        this->menu->lboard_button.label_texture, NULL, &this->menu->lboard_button.rect);

    SDL_RenderCopy(this->renderer, this->splash_title, NULL,
        &(SDL_Rect){this->width/2.0f - 400, 0, 800, 240});

    SDL_RenderPresent(this->renderer);
}
void Carcassone__Lboard__render(Carcassone* this)
{
    SDL_RenderClear(this->renderer);
    SDL_RenderPresent(this->renderer);
}
void Carcassone__Game__render(Carcassone* this)
{
    SDL_SetRenderDrawColor(this->renderer, 102, 102, 153, 255);
    SDL_RenderClear(this->renderer);

    Carcassone__render_board(this);
    Carcassone__render_splash_title(this);
    //Carcassone__render_player_stats(this);
    Carcassone__render_drawn_tile(this);

    SDL_RenderPresent(this->renderer);
}

void Carcassone__run(Carcassone* this)
{
    this->is_running = true;
    while(this->is_running) {
        Carcassone__handle_input(this);
        switch(this->state) {
            case MENU:
                Carcassone__Menu__render(this);
                break;
            case LBOARD:
                Carcassone__Lboard__render(this);
                break;
            case GAME:
                Carcassone__Game__render(this);
                break;
        }
    }
}
