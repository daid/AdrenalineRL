#pragma once

#include "r/vector2.h"

class ShipTile
{
public:
    int area_nr = -1;
    int connection_mask = 0;
};
extern r::Vector2<ShipTile> ship_tiles;

void generateShip();