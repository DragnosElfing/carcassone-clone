#ifndef CRCLONE_TILE_H
#define CRCLONE_TILE_H

#include <SDL2/SDL.h>

#include <stdbool.h>

#define TILE_SIZE 100
#define TILE_SIZE_SRC 64

typedef enum {
    NONE = 0U,
    FIELD,
    ROAD,
    CASTLE
} ConnectionType;

typedef enum {
    NORTH = 0U,
    EAST = 1U,
    SOUTH = 2U,
    WEST = 3U
} ConnectionDirection;

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

typedef struct {
    ConnectionType connections[4]; // a négy oldal: north, east, south, west
    TileType type;
    SDL_Point local_coords, global_coords; // bal felső sarok koord
    unsigned short rotation; // `unsigned int`, mert csak egész foknyi forgatást engedek
    bool rotatable;
} Tile;
void Tile__construct(Tile*, TileType, SDL_Point, SDL_Point);
bool Tile__point_in_tile(Tile*, SDL_Point);
void Tile__move_by(Tile*, float, float);
void Tile__rotate(Tile*);
void Tile__set_rotation(Tile*, unsigned short);
void Tile__set_type(Tile*, TileType, unsigned short);

typedef struct {
    SDL_Texture* tile_set;
} TilesetWrapper;
TilesetWrapper TilesetWrapper__construct(SDL_Renderer*);
void TilesetWrapper__destroy(TilesetWrapper*);
SDL_Rect TilesetWrapper__get_texture_rect_for(TilesetWrapper*, TileType);

#endif
