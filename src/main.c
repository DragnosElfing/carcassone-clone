#include "app.h"

int main(void)
{
    Carcassone* game = Carcassone__construct(1280, 960, "Carcassone másolat - Prog1 NHF");
    if(game != NULL) {
        Carcassone__run(game);
    }
    Carcassone__destroy(game);

    return 0;
}
