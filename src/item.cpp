#include "item.h"
#include "entity.h"
#include "guard.h"
#include "visualEffect.h"
#include "map.h"
#include "r/line.h"


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

int Item::range()
{
    return 12;
}

bool Item::fire(Entity* from, r::ivec2 target)
{
    if (type != Item::Type::RangedWeapon) return false;

    auto hit_position = from->pos();
    for(auto p : r::TraceLine(from->pos(), target)) {
        if (r::length(r::vec2(p - from->pos())) > range())
            break;
        hit_position = p;
        if (p.x < 0 || p.y < 0 || p.x >= map.size().x || p.y >= map.size().y)
            break;
        if (map[p].isSolid() && p != from->pos()) {
            break;
        }
    }
    Guard::makeNoise(from->pos(), 15.5, from->pos());
    new LineEffect(from->pos(), hit_position);
    return true;
}
