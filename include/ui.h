#ifndef CRCLONE_UI_H
#define CRCLONE_UI_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "game/tile.h"
#include "game/player.h"

/*! A játéktábla mérete (BOARD_SIZE * BOARD_SIZE) */
#define BOARD_SIZE 71

/*! Gomb */
typedef struct {
    // Címke.
    char* label;
    SDL_Texture* label_texture;
    SDL_Rect label_rect;

    // Címke betűtípusa.
    TTF_Font* used_font;

    // A gomb pozíciója (lokálisan és globálisan).
    SDL_Rect local_rect, global_rect;

    // Háttérszín.
    SDL_Color bg_color;
} Button;

/*! Szöveginput. (egy régi verzió miatt van külön a gombtól) */
typedef struct {
    Button prompt;
} Prompt;

/*! Menünézet. */
typedef struct {
    // Háttérkép.
    SDL_Texture* background;

    // A "menü menü."
    SDL_Rect button_container;
    Button start_button;
    Button lboard_button;
} MenuScreen;

/*! Dicsőséglistanézet. */
typedef struct {
    // Maga a dicsésglista (és adatai).
    Leaderboard* leaderboard;

    // Hibaüzenet hibás formátum esetén.
    char syntax_error_msg[128+1];

    // A rekordok egy textúrán.
    SDL_Texture* list_texture;
    
    // "Vissza" gomb.
    Button back_button;
} LeaderboardScreen;

/*! Játéknézet. */
typedef struct {
    // A ready_button meg lett e nyomva.
    bool is_ready;

    // Vége van e játéknak.
    bool is_game_over;

    // "Global state" a lenyomott nyílgombokhoz. (a tábla smooth mozgatása miatt)
    int held_arrow_keys[4];

    // A tábla felsősarkának koordinátái.
    SDL_FPoint board_offset;
    
    // Tábla.
    Tile** board;
    SDL_Texture* board_texture;

    // Húzott kártya.
    Tile* drawn_tile;

    // Pakli.
    CardPile* card_pile;
    size_t pile_index;
    
    // Pakliszámláló.
    SDL_Texture* pile_counter;

    // Kell e frissíteni a pakliszámlálót.
    bool update_counter;
    
    // Játékosok, referenciák.
    Player players[2];
    Player* curr_player;
    Player* winner;

    // A korona textúra.
    SDL_Texture* crown_texture;

    // Játékosneveknek a szöveginputjai.
    SDL_Texture* player_input_labels[2];
    Prompt player_name_inputs[2];
    Prompt* active_input;
    
    // "Ok" gomb, "Kör vége" gomb, "Felad" gomb.
    Button ready_button, end_turn_button, concede_button;
    
    // Az táblaatlasz.
    TilesetWrapper tileset_wrapper;
} GameScreen;

#endif