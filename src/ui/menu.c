#include <stdlib.h>

#include "utils.h"
#include "ui.h"
#include "app.h"

#ifdef _CRCLONE_DEBUG
    #include "debug/debugmalloc.h"
#endif

/**
 * @brief Létrehozza a menünézetet.
 *
 * Megjegyzés: A lefoglalt memória megfelelő felszabadításához meg kell hívni a `Carcassone__Menu__destroy` függvényt.
 *
 * @param this A `Carcassone` struktúra, amelynek létrehozza a menünézetét.
 */
void Carcassone__Menu__construct(Carcassone* this)
{
    this->menu_screen = malloc(sizeof(MenuScreen));
    this->menu_screen->background = create_SDL_texture_from_BMP(this->renderer, "./res/menu_bg.bmp");

    // A háttérkép transzparenciája miatt.
    SDL_SetRenderDrawBlendMode(this->renderer, SDL_BLENDMODE_BLEND);

    this->menu_screen->button_container = (SDL_Rect){this->width/2 - 400, 250, 800, 600};

    // Gombok létrehozása
    SDL_Rect start_button_rect = {
        this->menu_screen->button_container.x + this->menu_screen->button_container.w / 2 - 100,
        this->menu_screen->button_container.y + 100,
        200, 60
    };
    this->menu_screen->start_button = 
        Carcassone__Button__construct(this, this->default_font, "START", start_button_rect, (SDL_Color){COLOR_BLUE}, (SDL_Color){COLOR_WHITE}, true);

    SDL_Rect lboard_button_rect = {
        this->menu_screen->button_container.x + this->menu_screen->button_container.w / 2 - 200,
        this->menu_screen->button_container.y + 200,
        400, 60
    };
    this->menu_screen->lboard_button = 
        Carcassone__Button__construct(this, this->default_font, "DICSŐSÉGLISTA", lboard_button_rect, (SDL_Color){COLOR_BLUE}, (SDL_Color){COLOR_WHITE}, true);
}

/**
 * @brief Felszabadítja a megadott `Carcassone` struktúrához tartozó `MenuScreen` által lefoglalt memóriát.
 *
 * @param this A `Carcassone` struktúra, aminek a lefoglalt memóriáját fel kell szabadítani.
 */
void Carcassone__Menu__destroy(Carcassone* this)
{
    destroy_SDL_Texture(this->menu_screen->background);
    Carcassone__Button__destroy(this, &this->menu_screen->start_button);
    Carcassone__Button__destroy(this, &this->menu_screen->lboard_button);
    free(this->menu_screen);
}

void Carcassone__Menu__handle_input(Carcassone* this)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch(event.type) {
        case SDL_QUIT:
            this->is_running = false;
            break;
        case SDL_MOUSEBUTTONDOWN:
            if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->menu_screen->start_button.global_rect)) {
                Carcassone__switch_state(this, GAME);
            }
            if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->menu_screen->lboard_button.global_rect)) {
                Carcassone__switch_state(this, LEADERBOARD);
            }
            break;
        default:
            break;
    }
}

/**
 * @brief Menünézet megjelenítése.
 *
 * @param this A `Carcassone` struktúra, amihez a menü tartozik.
 */
void Carcassone__Menu__render(Carcassone* this)
{
    SDL_RenderClear(this->renderer);

    // Háttér.
    SDL_RenderCopy(this->renderer, this->menu_screen->background, NULL, NULL);
    SDL_SetRenderDrawColor(this->renderer, 235, 235, 225, 100);
    SDL_RenderFillRect(this->renderer, NULL);

    Carcassone__Button__render(this, &this->menu_screen->start_button);
    Carcassone__Button__render(this, &this->menu_screen->lboard_button);

    Carcassone__render_splash_title(this, &(SDL_Rect){this->width/2 - 400, 0, 800, 240});
}