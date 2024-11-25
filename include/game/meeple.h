#ifndef CRCLONE_MEEPLE_H
#define CRCLONE_MEEPLE_H

#include <stdbool.h>
#include <SDL2/SDL.h>

/*! Alattvaló. */
typedef struct {
    // Le van e helyezve.
    bool is_placed;

    // Táblakoordináták.
    // Ha !is_placed, akkor akár invalid, "szemét" értéket is tárolhat.
    int x, y;

    // Kinézete. (játékosoként változtatható).
    SDL_Texture* texture;
} Meeple;
Meeple Meeple__construct(SDL_Texture*);
void Meeple__render(SDL_Renderer*, unsigned int, SDL_Color);
void Meeple__destroy(Meeple*);

#endif
