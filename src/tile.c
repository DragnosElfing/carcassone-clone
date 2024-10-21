#include "tile.h"
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
    Tile__change_type(this, type, this->rotation);
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

void Tile__set_rotation(Tile* this, int new_rotation)
{
    if(!this->rotatable) return;

    // TODO: no
    new_rotation %= 360;
    for(int r = this->rotation; r != new_rotation;) {
        r += 90;
        r %= 360;

        ConnectionType old_north = this->north;
        ConnectionType old_east = this->east;
        ConnectionType old_south = this->south;
        ConnectionType old_west = this->west;

        this->north = old_west;
        this->east = old_north;
        this->south = old_east;
        this->west = old_south;

    }
    this->rotation = new_rotation;

}

void Tile__change_type(Tile* this, TileType new_type, int new_rotation)
{
    this->type = new_type;
    switch(new_type) {
        case FIELD_CLOISTER_ROAD_S:
            this->north = FIELD;
            this->east = FIELD;
            this->south = ROAD;
            this->west = FIELD;
            break;
        case FIELD_CLOISTER_ROAD_NS:
            this->north = ROAD;
            this->east = FIELD;
            this->south = ROAD;
            this->west = FIELD;
            break;
        case FIELD_VILLAGE_ROAD_S:
            this->north = FIELD;
            this->east = FIELD;
            this->south = ROAD;
            this->west = FIELD;
            break;
        case FIELD_VILLAGE_ROAD_NS:
            this->north = ROAD;
            this->east = FIELD;
            this->south = ROAD;
            this->west = FIELD;
            break;
        case ROAD_NS:
            this->north = ROAD;
            this->east = FIELD;
            this->south = ROAD;
            this->west = FIELD;
            break;
        case ROAD_NW:
            this->north = ROAD;
            this->east = FIELD;
            this->south = FIELD;
            this->west = ROAD;
            break;
        case ROAD_NWE:
            this->north = ROAD;
            this->east = ROAD;
            this->south = FIELD;
            this->west = ROAD;
            break;
        case ROAD_NSWE:
            this->north = ROAD;
            this->east = ROAD;
            this->south = ROAD;
            this->west = ROAD;
            this->rotatable = false;
            break;
        case CASTLE_PANTHEON:
        case CASTLE_TOWN:
            this->north = CASTLE;
            this->east = CASTLE;
            this->south = CASTLE;
            this->west = CASTLE;
            this->rotatable = false;
            break;
        case CASTLE_TUNNEL:
            this->north = CASTLE;
            this->east = FIELD;
            this->south = CASTLE;
            this->west = FIELD;
            break;
        case CASTLE_CORNER_WALL:
            this->north = CASTLE;
            this->east = FIELD;
            this->south = FIELD;
            this->west = CASTLE;
            break;
        case CASTLE_CORNER_WALL_ROAD_BY:
            this->north = CASTLE;
            this->east = ROAD;
            this->south = ROAD;
            this->west = CASTLE;
            break;
        case CASTLE_CAP_WALL:
            this->north = CASTLE;
            this->east = FIELD;
            this->south = FIELD;
            this->west = FIELD;
            break;
        case CASTLE_CAP_WALL_ROAD_TO:
            this->north = CASTLE;
            this->east = FIELD;
            this->south = ROAD;
            this->west = FIELD;
            break;
        case CASTLE_CAP_WALL_ROAD_BY:
            this->north = CASTLE;
            this->east = ROAD;
            this->south = FIELD;
            this->west = ROAD;
            break;
        case CASTLE_SHIRT_WALL:
            this->north = CASTLE;
            this->east = CASTLE;
            this->south = FIELD;
            this->west = CASTLE;
            break;
        case CASTLE_SHIRT_WALL_ROAD_TO:
            this->north = CASTLE;
            this->east = CASTLE;
            this->south = ROAD;
            this->west = CASTLE;
            break;
        case EMPTY:
        default:
            this->north = NONE;
            this->east = NONE;
            this->south = NONE;
            this->west = NONE;
            break;
    }

    Tile__set_rotation(this, new_rotation);
}
