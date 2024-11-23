#ifndef CRCLONE_UI_H
#define CRCLONE_UI_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "game/tile.h"
#include "game/player.h"

// A játéktábla mérete (BOARD_SIZE * BOARD_SIZE)
#define BOARD_SIZE 71

// Gomb
typedef struct {
    SDL_Rect label_rect, local_rect, global_rect;
    SDL_Color bg_color;
    char* label;
    SDL_Texture* label_texture;
    TTF_Font* used_font;
} Button;

// Szöveginput (régi verzió miatt van külön a gombtól, s azzal is lehet implementálni, csak talán így szebb)
typedef struct {
    Button prompt;
} Prompt;

// Menü állapot
typedef struct {
    // Háttérkép
    SDL_Texture* background;

    // A "menü menü"
    SDL_Rect button_container;
    Button start_button;
    Button lboard_button;
} MenuScreen;

// Dicsőséglista állapot
typedef struct {
    Leaderboard* leaderboard;
    char syntax_error_msg[128+1];

    // A rekordok egy textúrán
    SDL_Texture* list_texture;
    
    // "Vissza" gomb
    Button back_button;
} LeaderboardScreen;

// Játék állapot
typedef struct {
    // A ready_button meg lett e nyomva
    bool is_ready;

    // Vége van e játéknak
    bool is_game_over;

    // A tábla smooth mozgatása miatt van egy "global state" a lenyomott nyilakhoz.
    int held_arrow_keys[4];

    // A tábla felsősarkának koordinátái
    SDL_FPoint board_offset;
    
    // Tábla
    Tile** board;
    SDL_Texture* board_texture;

    // Húzott kártya
    Tile* drawn_tile;

    // Pakli
    CardPile* card_pile;
    size_t pile_index;
    
    // Az első implementálásakor így lehetett normálisan megoldani (TEMP).
    SDL_Texture* pile_counter[PILE_SIZE];
    
    // Játékosok, referenciák
    Player players[2];
    Player* curr_player;
    Player* winner;
    SDL_Texture* crown_texture;

    // Játékosneveknek a szöveginputjai
    SDL_Texture* player_input_labels[2];
    Prompt player_name_inputs[2];
    Prompt* active_input;
    
    // Startgomb, "Kör vége" gomb, "Felad" gomb
    Button ready_button, end_turn_button, concede_button;
    
    TilesetWrapper tileset_wrapper;
} GameScreen;

#endif