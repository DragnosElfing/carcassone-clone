#include <stdlib.h>

#include "utils.h"
#include "ui.h"
#include "app.h"

#ifdef _CRCLONE_DEBUG
    #include "debug/debugmalloc.h"
#endif

/**
 * @brief Létrehozza a dicsőséglistanézetet.
 *
 * Megjegyzés: A lefoglalt memória megfelelő felszabadításához meg kell hívni a `Carcassone__Lboard__destroy` függvényt.
 *
 * @param this A `Carcassone` struktúra, amelynek létrehozza a dicsőséglistanézetét.
 */
void Carcassone__Lboard__construct(Carcassone* this)
{
    this->lboard_screen = malloc(sizeof(LeaderboardScreen));
    this->lboard_screen->leaderboard = Leaderboard__construct("res/data/records.dat");
    this->lboard_screen->list_texture = NULL;
    strcpy(this->lboard_screen->syntax_error_msg, "Hibás fájlformátum!");

    // "Vissza" gomb.
    this->lboard_screen->back_button = 
        Carcassone__Button__construct(this, this->small_font, "VISSZA", (SDL_Rect){this->width - 150, 10, 140, 50}, 
        (SDL_Color){COLOR_WHITE}, (SDL_Color){0, 0, 0, 255}, true);

    Carcassone__Lboard__init_list_texture(this);
}

/**
 * @brief Felszabadítja a megadott `Carcassone` struktúrához tartozó `LeaderboardScreen` által lefoglalt memóriát.
 *
 * @param this A `Carcassone` struktúra, aminek a lefoglalt memóriáját fel kell szabadítani.
 */
void Carcassone__Lboard__destroy(Carcassone* this)
{
    Leaderboard__destroy(this->lboard_screen->leaderboard);
    destroy_SDL_Texture(this->lboard_screen->list_texture);
    Carcassone__Button__destroy(this, &this->lboard_screen->back_button);
    free(this->lboard_screen);
}

/**
 * @brief Inputok kezelése dicsőglistanézetben.
 *
 * @param this A `Carcassone` struktúra, amihez tartozik a dicsőglistanézet.
 */
void Carcassone__Lboard__handle_input(Carcassone* this)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch(event.type) {
        case SDL_QUIT:
            this->is_running = false;
            break;
        case SDL_MOUSEBUTTONDOWN:
            // Gombnyomás.
            if(SDL_PointInRect(&(SDL_Point){event.button.x, event.button.y}, &this->lboard_screen->back_button.global_rect)) {
                Carcassone__switch_state(this, MENU);
            }
            break;
        default:
            break;
    }
}

/**
 * @brief Létrehozza a dicsőséglistanézethez a rekordokat.
 *
 * Akkor kell meghívni, ha frissül a rekordfájl.
 *
 * @param this A `Carcassone` struktúra, amelynek létrehozza a dicsőséglistanézetét.
 */
void Carcassone__Lboard__init_list_texture(Carcassone* this)
{
    // Nem néz ki sajnos a legjobban.

    // A rekordok.
    destroy_SDL_Texture(this->lboard_screen->list_texture);
    this->lboard_screen->list_texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
        1000, this->height - 200);
    SDL_SetRenderTarget(this->renderer, this->lboard_screen->list_texture);

    if(this->lboard_screen->leaderboard != NULL) {
        Leaderboard__sort(this->lboard_screen->leaderboard);
        
        char score_string[10+1]; // safe bet
        int w, h;
        // A top 5 rekord megjelenítése.
        for(size_t place = 0U; place < MIN(5, this->lboard_screen->leaderboard->entries_size); ++place) {
            // A rekordhoz tartozó játékos neve.
            char* curr_name = this->lboard_screen->leaderboard->entries[place].name;
            TTF_SizeUTF8(this->small_font, curr_name, &w, &h);
            SDL_Surface* name_surface = TTF_RenderUTF8_Blended(this->small_font, this->lboard_screen->leaderboard->entries[place].name,
                (SDL_Color){COLOR_WHITE});
            if(name_surface != NULL) {
                SDL_Texture* name = SDL_CreateTextureFromSurface(this->renderer, name_surface);
                SDL_RenderCopy(this->renderer, name, 
                    NULL, &(SDL_Rect){0, 160 * place, w, h});
                SDL_FreeSurface(name_surface);
                destroy_SDL_Texture(name);
            }
            
            // A rekord.
            sprintf(score_string, "%u", this->lboard_screen->leaderboard->entries[place].highscore);
            TTF_SizeUTF8(this->small_font, score_string, &w, &h);
            SDL_Surface* score_surface = TTF_RenderUTF8_Blended(this->default_font, score_string,
                (SDL_Color){COLOR_WHITE});
            if(score_surface != NULL) {
                SDL_Texture* score = SDL_CreateTextureFromSurface(this->renderer, score_surface);
                SDL_RenderCopy(this->renderer, score, 
                    NULL, &(SDL_Rect){0, 160 * place + h, w, h});
                SDL_FreeSurface(score_surface);
                destroy_SDL_Texture(score);
            }
        }
    } else {
        // Hibás formátum esetén.
        int w, h;
        TTF_SizeUTF8(this->small_font, this->lboard_screen->syntax_error_msg, &w, &h);
        SDL_Surface* msg_surface = TTF_RenderUTF8_Blended(this->default_font, this->lboard_screen->syntax_error_msg,
            (SDL_Color){COLOR_WHITE});
        if(msg_surface != NULL) {
            SDL_Texture* error_msg = SDL_CreateTextureFromSurface(this->renderer, msg_surface);
            SDL_RenderCopy(this->renderer, error_msg, NULL, &(SDL_Rect){100, 100, w, h});
            SDL_FreeSurface(msg_surface);
            destroy_SDL_Texture(error_msg);
        }
    }

    SDL_SetRenderTarget(this->renderer, NULL);
    SDL_SetTextureBlendMode(this->lboard_screen->list_texture, SDL_BLENDMODE_BLEND);
}

/**
 * @brief Dicsőséglistanézet megjelenítése.
 *
 * @param this A `Carcassone` struktúra, amihez a dicsőséglista tartozik.
 */
void Carcassone__Lboard__render(Carcassone* this)
{
    SDL_SetRenderDrawColor(this->renderer, COLOR_BG);
    SDL_RenderClear(this->renderer);

    Carcassone__Button__render(this, &this->lboard_screen->back_button);

    SDL_RenderCopy(this->renderer, this->splash_title, NULL,
        &(SDL_Rect){this->width/2 - 400, 0, 800, 240});

    SDL_RenderCopy(this->renderer, this->lboard_screen->list_texture, NULL,
        &(SDL_Rect){this->width/2 - 500, 250, 1000, 700});
}
