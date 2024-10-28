#include "app.h"

/**
 * @brief The main entry point of the entire program.
 *
 * Constructs, inits the game loop for the game. Also destroys it on exit.
 *
 * @return 0
 */
int main(void)
{
    Carcassone* game = Carcassone__construct(1280, 960, "Carcassone másolat - Prog1 NHF");
    if(game != NULL) {
        Carcassone__run(game);
    }
    Carcassone__destroy(game);

    return 0;
}
