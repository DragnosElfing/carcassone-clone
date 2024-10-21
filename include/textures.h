#ifndef CRCLONE_TEXTURES_H
#define CRCLONE_TEXTURES_H

#include "tile.h"
#include <SDL2/SDL_render.h>

typedef struct {
    SDL_Texture* tile_set;
} TilesetWrapper;

TilesetWrapper TilesetWrapper__construct(SDL_Renderer*);
void TilesetWrapper__destroy(TilesetWrapper*);
SDL_Rect TilesetWrapper__get_texture_rect_for(TilesetWrapper*, TileType);

#endif