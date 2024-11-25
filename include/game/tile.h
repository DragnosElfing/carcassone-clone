#ifndef CRCLONE_TILE_H
#define CRCLONE_TILE_H

#include <stdbool.h>
#include <SDL2/SDL.h>

/*! Megjelenő mezőkártyák mérete. */
#define TILE_SIZE 100
/*! A texture atlas-ban egy mezőkártya mérete. */
#define TILE_SIZE_SRC 64
/*! A pakli maximális mérete. */
#define PILE_SIZE 71

/*! Kártyaoldal típusok. */
typedef enum {
    NONE = 0U,
    FIELD,
    ROAD,
    CASTLE
} ConnectionType;

/*! Alias-ok a kártyaoldalaknak. */
typedef enum {
    NORTH = 0U,
    EAST = 1U,
    SOUTH = 2U,
    WEST = 3U
} ConnectionDirection;

/*! Kártyatípusok. */
typedef enum {
    EMPTY = -1,
    FIELD_CLOISTER_ROAD_S = 0,
    FIELD_CLOISTER_ROAD_NS,
    FIELD_VILLAGE_ROAD_S,
    FIELD_VILLAGE_ROAD_NS,
    ROAD_NS,
    ROAD_NW,
    ROAD_NWE,
    ROAD_NSWE,
    CASTLE_TOWN,
    CASTLE_PANTHEON,
    CASTLE_TUNNEL,
    CASTLE_CORNER_WALL,
    CASTLE_CORNER_WALL_ROAD_BY,
    CASTLE_CAP_WALL,
    CASTLE_CAP_WALL_ROAD_TO,
    CASTLE_CAP_WALL_ROAD_BY,
    CASTLE_SHIRT_WALL,
    CASTLE_SHIRT_WALL_ROAD_TO,
    TILETYPE_SIZE__ // = 19, trick
} TileType;

/*! Mezőkártya. */
typedef struct {
    // Típusa.
    TileType type;

    // A négy oldal: north, east, south, west. (ehhez van a ConnectionDirection)
    ConnectionType connections[4];

    // Bal felső sarok koordinátái.
    SDL_FPoint local_coords, global_coords;

    // Táblán a koordináta.
    SDL_Point board_coords;

    // Elforgatás.
    // `unsigned int`, mert csak egész foknyi forgatás van engedve (sőt, gyakorlatban az is csak 90 fok).
    unsigned short rotation;
    
    // Egyes kártyákat, amelyeknek minden oldalkapcsolata azonos, nem lehet forgatni.
    bool rotatable;

    // Volt e már pontozva.
    bool is_scored;

    // Lehet e tenni még rá alattvalót. (kolostornál számít)
    bool is_expired;
} Tile;
void Tile__construct(Tile*, TileType, SDL_Point, SDL_FPoint);
bool Tile__point_in_tile(Tile*, SDL_FPoint);
void Tile__move_by(Tile*, float, float);
void Tile__rotate(Tile*);
void Tile__set_rotation(Tile*, unsigned short);
void Tile__set_type(Tile*, TileType, unsigned short);

/*! Kártyapakli stackként implementálva. */
typedef struct CardPile {
    // A tárolt kártyatípus.
    TileType card;
    
    // Az "alatta" levő kártya.
    struct CardPile* next;
} CardPile;
CardPile* CardPile__construct(void);
CardPile* CardPile__pop(CardPile*, TileType*);
CardPile* CardPile__push(CardPile*, TileType);
void CardPile__destroy(CardPile*);

/*! Egy segédstruktúra a textúra atlasz tárolásához. */
typedef struct {
    // A tárolt atlasz.
    SDL_Texture* tile_set;
} TilesetWrapper;
TilesetWrapper TilesetWrapper__construct(SDL_Renderer*);
void TilesetWrapper__destroy(TilesetWrapper*);
SDL_Rect get_texture_rect_for(TileType);

#endif
