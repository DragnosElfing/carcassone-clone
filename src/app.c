#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "utils.h"
#include "app.h"

#ifdef _CRCLONE_DEBUG
    #include "debug/debugmalloc.h"
#endif

/**
 * @brief Initicializálja az összes nézetet, SDL és TTF kontextusokat.
 *
 * Megjegyzés: A lefoglalt memória megfelelő felszabadításához meg kell hívni a Carcassone__destroy(Carcassone*) függvényt.
 *
 * @param width Az ablak szélessége.
 * @param height Az ablak magassága.
 * @param title Az ablak címe.
 * @return Pointer az újonnan létrehozott Carcassone structra.
 */
Carcassone* Carcassone__construct(int width, int height, char const* title)
{
    Carcassone* new_app = malloc(sizeof(Carcassone));
    new_app->width = width;
    new_app->height = height;
    new_app->is_running = false;
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

    new_app->window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
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

    new_app->default_font = TTF_OpenFont("res/fonts/Sedan_SC/sedan_sc.ttf", 48);
    new_app->small_font = TTF_OpenFont("res/fonts/Sedan_SC/sedan_sc.ttf", 36);
    if(new_app->default_font == NULL || new_app->small_font == NULL) {
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

    srand((unsigned int)(time(NULL) % SHRT_MAX));

    Carcassone__Menu__construct(new_app);
    Carcassone__Game__construct(new_app);
    Carcassone__Lboard__construct(new_app);

    new_app->smutex = SDL_CreateMutex();

    return new_app;
}

/**
 * @brief Felszabadítja a megadott Carcassone struktúra által lefoglalt memóriát.
 *
 * @param this A Carcassone struktúra, aminek a lefoglalt memóriáját fel kell szabadítani.
 */
void Carcassone__destroy(Carcassone* this)
{
    if(SDL_LockMutex(this->smutex) == -1) SDL_UnlockMutex(this->smutex);
    SDL_DestroyMutex(this->smutex);
    
    this->is_running = false;

    Carcassone__Menu__destroy(this);
    Carcassone__Game__destroy(this);
    Carcassone__Lboard__destroy(this);

    if(this->window_icon != NULL)   SDL_FreeSurface(this->window_icon);
    destroy_SDL_Texture(this->splash_title);
    if(this->window != NULL)        SDL_DestroyWindow(this->window);
    if(this->renderer != NULL)      SDL_DestroyRenderer(this->renderer);
    if(this->default_font != NULL)  TTF_CloseFont(this->default_font);
    if(this->small_font != NULL)    TTF_CloseFont(this->small_font);
    if(TTF_WasInit() != 0)          TTF_Quit();
    if(SDL_WasInit(0) != 0)   SDL_Quit();

    free(this);
}

void Carcassone__switch_state(Carcassone* this, AppState new_state)
{
    if(this->state == new_state) return;

    this->state = new_state;
    switch(new_state) {
        case MENU:
            this->game_screen->is_ready = false;
            this->game_screen->is_game_over = false;
            break;
        case GAME:
            this->game_screen->is_ready = false;
            this->game_screen->is_game_over = false;
            Carcassone__Game__init_board(this);
            Carcassone__Game__init_pile(this);
            break;
        case LEADERBOARD:
            Carcassone__Lboard__init_list_texture(this);
            break;
    }
}

/**
 * @brief A splash cím renderelése.
 * 
 * @param this A Carcassone struktúra, ami tartalmazza az SDL kontextust.
 */
void Carcassone__render_splash_title(Carcassone* this, SDL_Rect* dst_rect)
{
    SDL_RenderCopy(this->renderer, this->splash_title, NULL, dst_rect);
}

/**
 * @brief A fő programciklus.
 * 
 * @param this A Carcassone struktúra, ami tartalmazza az SDL kontextust.
 */
void Carcassone__run(Carcassone* this)
{
    this->is_running = true;

    // Ez egy másik projektemből van átmásolva.
    Uint64 now;
    Uint64 last = 0U;
    float accumulator = 0U;
    while(this->is_running) {
        // Calculate delta time
        now = SDL_GetTicks64();
        accumulator += MIN((float)(now - last), 1000.0f / FPS);
        last = now;
        for(float delta; accumulator >= (delta = 1.0f / FPS); accumulator -= delta) {
            SDL_LockMutex(this->smutex);
            switch(this->state) {
            case MENU:
                Carcassone__Menu__handle_input(this);
                Carcassone__Menu__render(this);
                break;
            case LEADERBOARD:
                Carcassone__Lboard__handle_input(this);
                Carcassone__Lboard__render(this);
                break;
            case GAME:
                Carcassone__Game__handle_input(this, delta);
                Carcassone__Game__render(this);
                break;
            }
            SDL_UnlockMutex(this->smutex);
            SDL_RenderPresent(this->renderer);
        }

    }
}
