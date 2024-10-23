#ifndef CRCLONE_TILE_H
#define CRCLONE_TILE_H

#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <stdbool.h>
#include "meeple.h"

#define TILE_SIZE 100
#define TILE_SIZE_SRC 64

typedef enum {
    NONE = 0U,
    CASTLE,
    ROAD,
    FIELD
} ConnectionType;

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
    TILETYPE_SIZE__ // = 19, jó kis trükk
} TileType;

typedef struct {
    ConnectionType north, east, south, west;
    TileType type;
    Meeple* meeples;
    size_t meeples_size;
    SDL_Point local_coords, global_coords;
    int rotation; // `int`, mert csak egész foknyi forgatást engedek
    bool rotatable;
} Tile;

void Tile__construct(Tile*, TileType, SDL_Point, SDL_Point);
bool Tile__point_in_tile(Tile*, SDL_Point);
void Tile__move_by(Tile*, float, float);
void Tile__rotate(Tile*);
void Tile__set_rotation(Tile*, int);
void Tile__change_type(Tile*, TileType, int);

#endif
