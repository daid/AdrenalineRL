#pragma once

#include <string>
#include "r/frontend/renderer.h"


class Item
{
public:
    enum class Type {
        RangedWeapon,
        MeleeWeapon,
        Key,
    } type = Type::RangedWeapon;
    int subtype = 0;

    std::string name();
    void draw(r::frontend::Renderer& renderer, r::ivec2 position);
};
