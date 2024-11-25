#include <SDL2/SDL.h>

#include "utils.h"
#include "game/tile.h"

#ifdef _CRCLONE_DEBUG
    #include "debug/debugmalloc.h"
#endif

/**
 * @brief Létrehoz egy új `TilesetWrapper`-t.
 *
 * Singletonként kell használni a programban.
 * Megjegyzés: A lefoglalt memória megfelelő felszabadításához meg kell hívni a `TilesetWrapper__destroy` függvényt.
 *
 * @param renderer A renderer amivel a főablakra rajzolunk.
 * @return Az új atlasz wrapper.
 */
TilesetWrapper TilesetWrapper__construct(SDL_Renderer* renderer)
{
    TilesetWrapper new_tswrapper = {
        .tile_set = NULL
    };

    new_tswrapper.tile_set = create_SDL_texture_from_BMP(renderer, "res/tileset.bmp");
    if(new_tswrapper.tile_set == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ASSERT, "Nem sikerült a res/tileset.bmp beolvasása!\n%s", SDL_GetError());
        return new_tswrapper;
    }

    return new_tswrapper;
}

/**
 * @brief Felszabadítja a megadott `TilesetWrapper` struktúrában tárolt textúra atlaszt.
 *
 * @param this A `TilesetWrapper`, aminek a lefoglalt memóriáját fel kell szabadítani.
 */
void TilesetWrapper__destroy(TilesetWrapper* this)
{
    if(this != NULL) {
        destroy_SDL_Texture(this->tile_set);
    }
}

/**
 * @brief A megadott kártyatípushoz tartozó textúra pozícióját számolja ki az atlaszon.
 *
 * @param type A mezőkártya típusa, amelynek a textúráját szeretnénk.
 * @return A textúra pozíciója az atlaszon.
 */
SDL_Rect get_texture_rect_for(TileType type)
{
    TileType type_index = type;

    // Direkt vannak a `TileType` enumban a típusok olyen sorrendben.
    SDL_Rect rect;
    rect.x = (type_index % (TILETYPE_SIZE__ / 2)) * TILE_SIZE_SRC;
    rect.y = (type_index / (TILETYPE_SIZE__ / 2)) * TILE_SIZE_SRC;
    rect.w = TILE_SIZE_SRC;
    rect.h = TILE_SIZE_SRC;

    return rect;
}
