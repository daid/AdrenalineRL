#include "map.h"
#include "entity.h"


r::Vector2<Tile> map;


bool Tile::isSolid()
{
    if (type == Type::Wall) return true;
    return entity != nullptr;
}

bool Tile::blocksVision()
{
    if (type == Type::Wall) return true;
    if (entity) return entity->blocksVision();
    return false;
}