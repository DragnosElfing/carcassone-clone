#ifndef CRCLONE_MEEPLE_H
#define CRCLONE_MEEPLE_H

#include "player.h"

typedef struct {
    Player* owner;
    SDL_Texture* texture;
} Meeple;

#endif
