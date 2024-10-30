#ifndef CRCLONE_APP_H
#define CRCLONE_APP_H

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "tile.h"
#include "player.h"
#include "ui.h"

#define FPS 60
#define MIN(x, y) ((x < y)?x:y)
#define COLOR_BLUE 25, 30, 91, 255
#define COLOR_WHITE 255, 255, 255, 255
#define DBG_LOG(x, ...) SDL_LogDebug(SDL_LOG_CATEGORY_CUSTOM, x, ##__VA_ARGS__)

typedef enum
{
    MENU,
    GAME,
    LEADERBOARD
} AppState;

typedef struct
{
    int width, height;
    bool is_running;

    AppState state;
    MenuScreen* menu_screen;
    LeaderboardScreen* lboard_screen;
    GameScreen* game_screen;

    SDL_Window* window;
    SDL_Surface* window_icon;
    SDL_Texture* splash_title;
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
