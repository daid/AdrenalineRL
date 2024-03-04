#pragma once

#include "r/vector2.h"

class Entity;
class Item;
class Tile
{
public:
    enum class Type {
        Void,
        Wall,
        Floor,
    } type = Type::Void;
    enum class EnemySight {
        None,
        Minor,
        Major,
    } enemy_sight = EnemySight::None;
    bool player_visible = false;
    Entity* entity = nullptr;
    Entity* second_entity = nullptr; // Used for open doors.
    std::vector<Item*> items;

    bool isSolid();
    bool blocksVision();
};

extern r::Vector2<Tile> map;
