#include "item.h"

std::string Item::name()
{
    switch(type) {
    case Type::RangedWeapon: return "Rifle";
    case Type::MeleeWeapon: return "Knife";
    case Type::Key: return "Key";
    }
    return "Unknown";
}

void Item::draw(r::frontend::Renderer& renderer, r::ivec2 position)
{
    switch(type) {
    case Type::RangedWeapon: renderer.draw(position, '{', {.2, .2, 1}); break;
    case Type::MeleeWeapon: renderer.draw(position, '/', {.2, .2, 1}); break;
    case Type::Key: renderer.draw(position, '0' + subtype, {.9, .9, .1}); break;
    }
}