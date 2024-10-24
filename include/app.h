#ifndef CRCLONE_APP_H
#define CRCLONE_APP_H

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "tile.h"
#include "textures.h"

#define PILE_SIZE 71
#define GRID_SIZE 10
//#define FPS 60

typedef enum
{
    MENU,
    GAME,
    LBOARD
} AppState;

typedef struct
{
    SDL_Rect rect;
    char const* label;
    SDL_Texture* label_texture;
} Button;

typedef struct
{
    SDL_Texture* background;
    SDL_FRect button_container;
    Button start_button;
    Button lboard_button;
} Menu;

typedef struct
{
    SDL_Texture* list_texture;
    Button back_button;
} Lboard;

typedef struct
{
    int width, height;
    bool is_running;

    SDL_Point map_offset;
    Tile** board;
    Tile* drawn_tile;
    Tile* card_pile[PILE_SIZE];
    size_t pile_index;
    Player players[2];
    unsigned short curr_player_index;
    SDL_Texture* pile_counter[PILE_SIZE];
    TilesetWrapper tileset_wrapper;
    Leaderboard* leaderboard;

    AppState state;
    Menu* menu;
    Lboard* lboard;

    SDL_Window* window;
    SDL_Surface* window_icon;
    SDL_Texture* splash_title;
    SDL_Texture* board_texture;
    SDL_Renderer* renderer;
    TTF_Font* default_font;
} Carcassone;

Carcassone* Carcassone__construct(int, int, char const*);
void Carcassone__destroy(Carcassone*);
void Carcassone__Menu__construct(Carcassone*);
void Carcassone__Menu__destroy(Carcassone*);
void Carcassone__Lboard__construct(Carcassone*);
void Carcassone__Lboard__destroy(Carcassone*);

void Carcassone__run(Carcassone*);
void Carcassone__Menu__render(Carcassone*);
void Carcassone__Game__render(Carcassone*);
void Carcassone__Lboard__render(Carcassone*);
void Carcassone__handle_input(Carcassone*);

void Carcassone__render_board(Carcassone*);
void Carcassone__render_splash_title(Carcassone*);
void Carcassone__render_drawn_tile(Carcassone*);
void Carcassone__render_player_stats(Carcassone*);

void Carcassone__init_players(Carcassone*);
void Carcassone__init_pile(Carcassone*);
void Carcassone__init_board(Carcassone*);
void Carcassone__init_counter(Carcassone*);
void Carcassone__move_board(Carcassone*, SDL_KeyCode);
void Carcassone__draw_new(Carcassone*);
bool Carcassone__check_surrounding_tiles(Carcassone*, SDL_Point);


#endif
