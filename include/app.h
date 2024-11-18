#ifndef CRCLONE_APP_H
#define CRCLONE_APP_H

#include <SDL2/SDL_scancode.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>

#include "game/tile.h"
#include "game/player.h"
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
    TTF_Font* default_font, * small_font;
} Carcassone;

Carcassone* Carcassone__construct(int, int, char const*);
void Carcassone__destroy(Carcassone*);
void Carcassone__switch_state(Carcassone*, AppState);
void Carcassone__Menu__construct(Carcassone*);
void Carcassone__Menu__destroy(Carcassone*);
void Carcassone__Lboard__construct(Carcassone*);
void Carcassone__Lboard__destroy(Carcassone*);
void Carcassone__Game__construct(Carcassone*);
void Carcassone__Game__destroy(Carcassone*);
void Carcassone__run(Carcassone*);
void Carcassone__Menu__render(Carcassone*);
void Carcassone__Game__render(Carcassone*);
void Carcassone__Lboard__render(Carcassone*);
void Carcassone__handle_input(Carcassone*);
void Carcassone__render_board(Carcassone*);
void Carcassone__render_splash_title(Carcassone*);
void Carcassone__render_drawn_tile(Carcassone*);
void Carcassone__render_player_stats(Carcassone*);
void Carcassone__render_meeples(Carcassone*);
void Carcassone__indicate_possible_placements(Carcassone*);
void Carcassone__init_players(Carcassone*);
void Carcassone__init_pile(Carcassone*);
void Carcassone__init_board(Carcassone*);
void Carcassone__init_counter(Carcassone*);
void Carcassone__move_board(Carcassone*, SDL_Scancode);
void Carcassone__draw_new(Carcassone*);
bool Carcassone__check_surrounding_tiles(Carcassone*, SDL_Point);
void Carcassone__check_scorable_constructs(Carcassone*);
void Carcassone__calculate_scores_for_cloister(Carcassone*, Tile*);
void Carcassone__show_finish_screen(Carcassone*);
Button Carcassone__Button__construct(Carcassone*, TTF_Font*, char*, SDL_Rect, SDL_Color, SDL_Color, bool);
bool Carcassone__Button__hover(Carcassone*, Button*, SDL_Point);
void Carcassone__Button__render(Carcassone*, Button*);
void Carcassone__Button__destroy(Carcassone*, Button*);
Prompt Carcassone__Prompt__construct(Carcassone*, TTF_Font*, char*, SDL_Rect, SDL_Color, SDL_Color);
void Carcassone__Prompt__edit(Carcassone*, Prompt*, char*, bool);
void Carcassone__Prompt__render(Carcassone*, Prompt*);
void Carcassone__Prompt__destroy(Carcassone*, Prompt*);

//static void Carcassone__render_text_to(Carcassone*, SDL_Texture*, char const*);
size_t get_utf8_length(char*);

#endif
