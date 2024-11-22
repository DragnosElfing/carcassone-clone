#ifndef CRCLONE_MEEPLE_H
#define CRCLONE_MEEPLE_H

#include <stdbool.h>
#include <SDL2/SDL.h>

typedef struct {
    // Le van e helyezve
    bool is_placed;

    // Ha !is_placed, akkor akár invalid, "szemét" értéket is tárolhat.
    int x, y;

    // Mindegyik alattvalónak ugyanaz a textúrája valójában.
    SDL_Texture* texture;
} Meeple;
Meeple Meeple__construct(SDL_Renderer*);
void Meeple__render(SDL_Renderer*, unsigned int, SDL_Color);
void Meeple__destroy(Meeple*);

#endif
