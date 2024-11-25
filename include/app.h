#ifndef CRCLONE_APP_H
#define CRCLONE_APP_H

#include <bits/pthreadtypes.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "game/tile.h"
#include "game/player.h"
#include "ui.h"

/*! Cél FPS. */
#define FPS 60
/*! Egyszerű minimum meghatározó. */
#define MIN(x, y) ((x < y) ? x : y)
/*! Színek. */
#define COLOR_BLUE 25, 30, 91, 255
#define COLOR_LIGHTBLUE 153, 204, 255, 255
#define COLOR_WHITE 255, 255, 255, 255
#define COLOR_RED 255, 0, 0, 255
#define COLOR_SALMON 255, 145, 164, 255
#define COLOR_BG 102, 102, 153, 255
/*! Development alatt használt logger. */
#define DBG_LOG(x, ...) SDL_LogDebug(SDL_LOG_CATEGORY_CUSTOM, x, ##__VA_ARGS__)

/*! A program állapotai: menü, játék, dicsőséglista. */
typedef enum {
    MENU,
    GAME,
    LEADERBOARD
} AppState;

/*! Magát a programot reprezentáló struktúra. */
typedef struct {
    // Ablak dimenziói.
    int width, height;

    // Fut e.
    bool is_running;

    // Az ablak.
    SDL_Window* window;

    // Ablakikon.
    SDL_Surface* window_icon;

    // Splash cím.
    SDL_Texture* splash_title;

    // A renderer.
    SDL_Renderer* renderer;

    // A használt betűtípusok.
    TTF_Font* default_font, * small_font;

    // Állapotok.
    AppState state;
    MenuScreen* menu_screen;
    LeaderboardScreen* lboard_screen;
    GameScreen* game_screen;

    // Mutex arra az esetre, mikor visszamegyünk a menübe game over-t követve.
    SDL_mutex* smutex;
} Carcassone;
Carcassone* Carcassone__construct(int, int, char const*);
void Carcassone__destroy(Carcassone*);
void Carcassone__switch_state(Carcassone*, AppState);
void Carcassone__run(Carcassone*);
void Carcassone__render_splash_title(Carcassone*, SDL_Rect*);

void Carcassone__Menu__construct(Carcassone*);
void Carcassone__Menu__destroy(Carcassone*);
void Carcassone__Menu__render(Carcassone*);
void Carcassone__Menu__handle_input(Carcassone*);

void Carcassone__Lboard__construct(Carcassone*);
void Carcassone__Lboard__destroy(Carcassone*);
void Carcassone__Lboard__render(Carcassone*);
void Carcassone__Lboard__init_list_texture(Carcassone*);
void Carcassone__Lboard__handle_input(Carcassone*);

void Carcassone__Game__construct(Carcassone*);
void Carcassone__Game__destroy(Carcassone*);
void Carcassone__Game__handle_input(Carcassone*, float);
void Carcassone__Game__init_players(Carcassone*);
void Carcassone__Game__init_pile(Carcassone*);
void Carcassone__Game__init_board(Carcassone*);
void Carcassone__Game__init_counter(Carcassone*);
void Carcassone__Game__render(Carcassone*);
void Carcassone__Game__render_board(Carcassone*);
void Carcassone__Game__render_drawn_tile(Carcassone*);
void Carcassone__Game__render_player_stats(Carcassone*);
void Carcassone__Game__render_meeples(Carcassone*);
void Carcassone__Game__render_game_over(Carcassone*);
void Carcassone__Game__wrapup(Carcassone*, bool);
void Carcassone__Game__move_board(Carcassone*, float);
bool Carcassone__Game__draw_new(Carcassone*);
bool Carcassone__Game__check_names_valid(Carcassone*);
void Carcassone__Game__indicate_possible_placements(Carcassone*);
bool Carcassone__Game__check_surrounding_tiles(Carcassone*, SDL_Point);
void Carcassone__Game__calculate_points(Carcassone*);
bool Carcassone__Game__check_if_possible(Carcassone*);
void Carcassone__Game__calculate_scores_for_cloister(Carcassone*, Tile*);
void Carcassone__Game__calculate_scores_for_graph(Carcassone*, Tile*, ConnectionType);

Button Carcassone__Button__construct(Carcassone*, TTF_Font*, char*, SDL_Rect, SDL_Color, SDL_Color, bool);
void Carcassone__Button__destroy(Carcassone*, Button*);
void Carcassone__Button__render(Carcassone*, Button*);
bool Carcassone__Button__hover(Carcassone*, Button*, SDL_Point);

Prompt Carcassone__Prompt__construct(Carcassone*, TTF_Font*, char*, SDL_Rect, SDL_Color, SDL_Color);
void Carcassone__Prompt__destroy(Carcassone*, Prompt*);
void Carcassone__Prompt__render(Carcassone*, Prompt*);
void Carcassone__Prompt__edit(Carcassone*, Prompt*, char*, bool);

#endif
