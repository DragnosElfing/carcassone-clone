#ifndef CRCLONE_UI_H
#define CRCLONE_UI_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "game/tile.h"
#include "game/player.h"

#define PILE_SIZE 71
#define BOARD_SIZE 10

typedef struct
{
    SDL_Rect rect, global_rect;
    SDL_Color bg_color;
    char* label;
    SDL_Texture* label_texture;
} Button;

typedef struct
{
    Button prompt;
    bool is_active;
} Prompt;

typedef struct
{
    SDL_Texture* background;
    SDL_Rect button_container;
    Button start_button;
    Button lboard_button;
} MenuScreen;

typedef struct
{
    Leaderboard* leaderboard;
    SDL_Texture* list_texture;
    Button back_button;
} LeaderboardScreen;

typedef struct
{
    bool is_ready;
    SDL_Point board_offset;
    Tile** board;
    Tile* drawn_tile;
    Tile* card_pile[PILE_SIZE];
    size_t pile_index;
    Player players[2];
    Prompt player_name_inputs[2];
    Prompt* active_input;
    Button ready_button;
    Player* curr_player;
    SDL_Texture* pile_counter[PILE_SIZE];
    TilesetWrapper tileset_wrapper;
    SDL_Texture* board_texture;
    int held_arrow_keys[4];
} GameScreen;

#endif