#ifndef CRCLONE_MEEPLE_H
#define CRCLONE_MEEPLE_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "game/tile.h"

typedef struct {
    int x, y;
    Tile* tile;
    SDL_Texture* texture;
    bool is_placed;
} Meeple;
Meeple Meeple__construct(SDL_Renderer*);
void Meeple__place_on(Meeple*, unsigned int, unsigned int);
void Meeple__reclaim(Meeple*);
void Meeple__destroy(Meeple*);

#endif
