#include "textures.h"

TilesetWrapper TilesetWrapper__construct(SDL_Renderer* renderer)
{
    TilesetWrapper new_tswrapper;
    new_tswrapper.tile_set = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("res/tileset.bmp"));
    return new_tswrapper;
}

void TilesetWrapper__destroy(TilesetWrapper* this)
{
    if(this->tile_set != NULL) {
        SDL_DestroyTexture(this->tile_set);
    }
}

SDL_Rect TilesetWrapper__get_texture_rect_for(TilesetWrapper* this, TileType type)
{
    short type_index = type;

    SDL_Rect rect;
    rect.x = (type_index % (__TILETYPE_SIZE / 2)) * TILE_SIZE_SRC;
    rect.y = (type_index / (__TILETYPE_SIZE / 2)) * TILE_SIZE_SRC;
    rect.w = TILE_SIZE_SRC;
    rect.h = TILE_SIZE_SRC;

    return rect;
}