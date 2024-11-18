#ifndef CRCLONE_MEEPLE_H
#define CRCLONE_MEEPLE_H

#include <stdbool.h>
#include <SDL2/SDL.h>

typedef struct {
    int x, y;
    SDL_Texture* texture;
    bool is_placed;
} Meeple;
Meeple Meeple__construct(SDL_Renderer*);
void Meeple__destroy(Meeple*);

#endif
