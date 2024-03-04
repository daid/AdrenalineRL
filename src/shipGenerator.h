#pragma once

#include "r/vector2.h"

class ShipTile
{
public:
    int area_nr = -1;
    int connection_mask = 0;
};
class AreaInfo
{
public:
    std::vector<r::ivec2> tiles;
};
extern r::Vector2<ShipTile> ship_tiles;
extern std::vector<AreaInfo> area_info;

void generateShip();
