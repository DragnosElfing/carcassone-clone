#include <math.h>
#include "app.h"
#include "game/tile.h"
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>

void Tile__construct(Tile* this, TileType type, SDL_Point lcoords, SDL_Point offset)
{
    this->local_coords = (SDL_Point){lcoords.x * TILE_SIZE, lcoords.y * TILE_SIZE};
    this->global_coords = (SDL_Point){this->local_coords.x + offset.x, this->local_coords.y + offset.y};

    this->rotatable = true;
    this->rotation = 0;
    Tile__set_type(this, type, this->rotation);
}

bool Tile__point_in_tile(Tile* this, SDL_Point pt)
{
    return this->global_coords.x < pt.x && pt.x <= this->global_coords.x + TILE_SIZE
        && this->global_coords.y < pt.y && pt.y <= this->global_coords.y + TILE_SIZE;
}

void Tile__move_by(Tile* this, float mvx, float mvy)
{
    this->local_coords = (SDL_Point){this->local_coords.x + mvx * TILE_SIZE, this->local_coords.y + mvy * TILE_SIZE};
    this->global_coords = (SDL_Point){this->global_coords.x + mvx * TILE_SIZE, this->global_coords.y + mvy * TILE_SIZE};
}

void Tile__rotate(Tile* this)
{
    Tile__set_rotation(this, this->rotation + 90);
}

void Tile__set_rotation(Tile* this, unsigned short new_rotation)
{
    if(!this->rotatable) return;

    ConnectionType prev_connections[4];

    // TODO: no
    new_rotation %= 360;
    unsigned int r = this->rotation;
    while(r != new_rotation) {
        r += 90;
        r %= 360;

        for(unsigned int dir = NORTH; dir <= WEST; ++dir) {
            prev_connections[dir] = this->connections[dir];
        }
        for(int dir = NORTH; dir <= WEST; ++dir) {
            this->connections[dir] = (dir - 1) < 0 ? prev_connections[3] : prev_connections[dir - 1];
        }
        
    }

    this->rotation = new_rotation;
}

void Tile__set_type(Tile* this, TileType new_type, unsigned short new_rotation)
{
    this->type = new_type;
    switch(new_type) {
        case FIELD_CLOISTER_ROAD_S:
            this->connections[NORTH] = FIELD;
            this->connections[EAST] = FIELD;
            this->connections[SOUTH] = ROAD;
            this->connections[WEST] = FIELD;
            break;
        case FIELD_CLOISTER_ROAD_NS:
            this->connections[NORTH] = ROAD;
            this->connections[EAST] = FIELD;
            this->connections[SOUTH] = ROAD;
            this->connections[WEST] = FIELD;
            break;
        case FIELD_VILLAGE_ROAD_S:
            this->connections[NORTH] = FIELD;
            this->connections[EAST] = FIELD;
            this->connections[SOUTH] = ROAD;
            this->connections[WEST] = FIELD;
            break;
        case FIELD_VILLAGE_ROAD_NS:
            this->connections[NORTH] = ROAD;
            this->connections[EAST] = FIELD;
            this->connections[SOUTH] = ROAD;
            this->connections[WEST] = FIELD;
            break;
        case ROAD_NS:
            this->connections[NORTH] = ROAD;
            this->connections[EAST] = FIELD;
            this->connections[SOUTH] = ROAD;
            this->connections[WEST] = FIELD;
            break;
        case ROAD_NW:
            this->connections[NORTH] = ROAD;
            this->connections[EAST] = FIELD;
            this->connections[SOUTH] = FIELD;
            this->connections[WEST] = ROAD;
            break;
        case ROAD_NWE:
            this->connections[NORTH] = ROAD;
            this->connections[EAST] = ROAD;
            this->connections[SOUTH] = FIELD;
            this->connections[WEST] = ROAD;
            break;
        case ROAD_NSWE:
            this->connections[NORTH] = ROAD;
            this->connections[EAST] = ROAD;
            this->connections[SOUTH] = ROAD;
            this->connections[WEST] = ROAD;
            this->rotatable = false;
            break;
        case CASTLE_PANTHEON:
        case CASTLE_TOWN:
            this->connections[NORTH] = CASTLE;
            this->connections[EAST] = CASTLE;
            this->connections[SOUTH] = CASTLE;
            this->connections[WEST] = CASTLE;
            this->rotatable = false;
            break;
        case CASTLE_TUNNEL:
            this->connections[NORTH] = CASTLE;
            this->connections[EAST] = FIELD;
            this->connections[SOUTH] = CASTLE;
            this->connections[WEST] = FIELD;
            break;
        case CASTLE_CORNER_WALL:
            this->connections[NORTH] = CASTLE;
            this->connections[EAST] = FIELD;
            this->connections[SOUTH] = FIELD;
            this->connections[WEST] = CASTLE;
            break;
        case CASTLE_CORNER_WALL_ROAD_BY:
            this->connections[NORTH] = CASTLE;
            this->connections[EAST] = ROAD;
            this->connections[SOUTH] = ROAD;
            this->connections[WEST] = CASTLE;
            break;
        case CASTLE_CAP_WALL:
            this->connections[NORTH] = CASTLE;
            this->connections[EAST] = FIELD;
            this->connections[SOUTH] = FIELD;
            this->connections[WEST] = FIELD;
            break;
        case CASTLE_CAP_WALL_ROAD_TO:
            this->connections[NORTH] = CASTLE;
            this->connections[EAST] = FIELD;
            this->connections[SOUTH] = ROAD;
            this->connections[WEST] = FIELD;
            break;
        case CASTLE_CAP_WALL_ROAD_BY:
            this->connections[NORTH] = CASTLE;
            this->connections[EAST] = ROAD;
            this->connections[SOUTH] = FIELD;
            this->connections[WEST] = ROAD;
            break;
        case CASTLE_SHIRT_WALL:
            this->connections[NORTH] = CASTLE;
            this->connections[EAST] = CASTLE;
            this->connections[SOUTH] = FIELD;
            this->connections[WEST] = CASTLE;
            break;
        case CASTLE_SHIRT_WALL_ROAD_TO:
            this->connections[NORTH] = CASTLE;
            this->connections[EAST] = CASTLE;
            this->connections[SOUTH] = ROAD;
            this->connections[WEST] = CASTLE;
            break;
        case EMPTY:
        default:
            this->connections[NORTH] = NONE;
            this->connections[EAST] = NONE;
            this->connections[SOUTH] = NONE;
            this->connections[WEST] = NONE;
            break;
    }

    Tile__set_rotation(this, new_rotation);
}
