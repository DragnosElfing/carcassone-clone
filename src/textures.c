#include <SDL2/SDL.h>

#include "tile.h"

TilesetWrapper TilesetWrapper__construct(SDL_Renderer* renderer)
{
    TilesetWrapper new_tswrapper;
    
    SDL_Surface* tileset_img = SDL_LoadBMP("res/tileset.bmp");
    if(tileset_img == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ASSERT, "Nem sikerült a res/tileset.bmp beolvasása!\n%s", SDL_GetError());
        return new_tswrapper;
    }
    new_tswrapper.tile_set = SDL_CreateTextureFromSurface(renderer, tileset_img);
    SDL_FreeSurface(tileset_img);

    return new_tswrapper;
}

void TilesetWrapper__destroy(TilesetWrapper* this)
{
    if(this->tile_set != NULL) SDL_DestroyTexture(this->tile_set);
}

SDL_Rect TilesetWrapper__get_texture_rect_for(TilesetWrapper* this, TileType type)
{
    short type_index = type;

    SDL_Rect rect;
    rect.x = (type_index % (TILETYPE_SIZE__ / 2)) * TILE_SIZE_SRC;
    rect.y = (type_index / (TILETYPE_SIZE__ / 2)) * TILE_SIZE_SRC;
    rect.w = TILE_SIZE_SRC;
    rect.h = TILE_SIZE_SRC;

    return rect;
}
