#include <SDL2/SDL_log.h>
#include "app.h"

/**
 * @brief A program entry pointja.
 * 
 * Létrehozza és inicializálja a programciklust, illetve biztosítja a lefoglalt memória helyes felszabadulását.
 *
 * @return 0
 */
int main(void)
{
    #ifdef _CRCLONE_DEBUG
        SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    #else
        SDL_LogSetAllPriority(SDL_LOG_PRIORITY_CRITICAL);
    #endif

    Carcassone* game = Carcassone__construct(1280, 960, "Carcassone másolat - Prog1 NHF");
    if(game != NULL) {
        Carcassone__run(game);
        Carcassone__destroy(game);
    }

    return 0;
}
