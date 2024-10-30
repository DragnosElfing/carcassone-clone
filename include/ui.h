#ifndef CRCLONE_UI_H
#define CRCLONE_UI_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "tile.h"
#include "player.h"

#define PILE_SIZE 71
#define BOARD_SIZE 10

typedef struct
{
    SDL_Rect rect;
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
    SDL_FRect button_container;
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
    SDL_Point map_offset;
    Tile** board;
    Tile* drawn_tile;
    Tile* card_pile[PILE_SIZE];
    size_t pile_index;
    Player players[2];
    Prompt player_name_inputs[2];
    Player* curr_player;
    SDL_Texture* pile_counter[PILE_SIZE];
    TilesetWrapper tileset_wrapper;
    SDL_Texture* board_texture;
} GameScreen;

#endif