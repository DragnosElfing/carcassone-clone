#include <math.h>
#include <SDL2/SDL.h>

#include "app.h"
#include "game/tile.h"

#ifdef _CRCLONE_DEBUG
    #include "debug/debugmalloc.h"
#endif

CardPile* CardPile__construct(void)
{
    CardPile* new_cardpile = malloc(sizeof(CardPile));
    if(new_cardpile == NULL) return NULL;

    new_cardpile->card = EMPTY;
    new_cardpile->next = NULL;

    return new_cardpile;
}
void CardPile__destroy(CardPile* this)
{
    if(this == NULL) return;

    CardPile__destroy(this->next);
    free(this);
}
CardPile* CardPile__pop(CardPile* this, TileType* popped)
{
    if(this == NULL) {
        *popped = EMPTY;
        return NULL;
    }

    *popped = this->card;
    CardPile* new_top = this->next;
    free(this);

    return new_top;
}
CardPile* CardPile__push(CardPile* this, TileType new_type)
{
    CardPile* new_top = CardPile__construct();
    if(new_top == NULL) return NULL;

    new_top->card = new_type;
    new_top->next = this;

    return new_top;
}


void Tile__construct(Tile* this, TileType type, SDL_Point board_coords, SDL_FPoint offset)
{
    if(this == NULL) return;

    this->board_coords = board_coords;
    this->local_coords = (SDL_FPoint){board_coords.x * TILE_SIZE, board_coords.y * TILE_SIZE};
    this->global_coords = (SDL_FPoint){this->local_coords.x + offset.x, this->local_coords.y + offset.y};
    this->is_scored = false;
    this->is_expired = false;

    this->rotatable = true;
    this->rotation = 0;
    Tile__set_type(this, type, this->rotation);
}

bool Tile__point_in_tile(Tile* this, SDL_FPoint pt)
{
    if(this == NULL) return false;

    // ? Lehetne kis módosítással `SDL_PointInRect`-et is használni.
    return this->global_coords.x < pt.x && pt.x <= this->global_coords.x + TILE_SIZE
        && this->global_coords.y < pt.y && pt.y <= this->global_coords.y + TILE_SIZE;
}

void Tile__move_by(Tile* this, float mvx, float mvy)
{
    if(this == NULL) return;
    
    this->local_coords = (SDL_FPoint){this->local_coords.x + mvx * TILE_SIZE, this->local_coords.y + mvy * TILE_SIZE};
    this->global_coords = (SDL_FPoint){this->global_coords.x + mvx * TILE_SIZE, this->global_coords.y + mvy * TILE_SIZE};
}

void Tile__rotate(Tile* this)
{
    if(this == NULL) return;

    Tile__set_rotation(this, this->rotation + 90);
}

void Tile__set_rotation(Tile* this, unsigned short new_rotation)
{
    if(this == NULL || !this->rotatable) return;

    ConnectionType prev_connections[4];

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
    if(this == NULL) return;

    this->type = new_type;
    // ? Esetleg egy config fájlt létre lehetne hozni ennek.
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
