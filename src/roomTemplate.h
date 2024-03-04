#pragma once

#include "r/vector2.h"
#include "map.h"


class RoomTemplate
{
public:
    r::Vector2<Tile::Type> tiles;

    RoomTemplate rotate();

    static void init();

    static std::unordered_map<int, std::vector<RoomTemplate>> templates;
};