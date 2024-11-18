#ifndef CRCLONE_UI_H
#define CRCLONE_UI_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "game/tile.h"
#include "game/player.h"

#define BOARD_SIZE 10

typedef struct
{
    SDL_Rect label_rect, local_rect, global_rect;
    SDL_Color bg_color;
    char* label;
    SDL_Texture* label_texture;
    TTF_Font* used_font;
} Button;

typedef struct
{
    Button prompt;
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
    int held_arrow_keys[4];
    SDL_FPoint board_offset;
    
    Tile** board;
    Tile* drawn_tile;
    CardPile* card_pile;
    size_t pile_index;
    
    Player players[2];
    Player* curr_player;

    Prompt player_name_inputs[2];
    Prompt* active_input;
    
    Button ready_button, end_turn_button, concede_button;
    
    SDL_Texture* pile_counter[PILE_SIZE];
    TilesetWrapper tileset_wrapper;
    SDL_Texture* board_texture;
} GameScreen;

#endif