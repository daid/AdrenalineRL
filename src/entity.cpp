#include "entity.h"
#include "map.h"


std::vector<Entity*> Entity::all;
static size_t next_id = 1;

Entity::Entity(r::ivec2 _position)
: position(_position)
{
    assert(position.x >= 0 && position.y >= 0 && position.x < map.size().x && position.y < map.size().y);
    map[position].entity = this;
    all.push_back(this);
    _id = next_id++;
}

Entity::~Entity()
{
    map[position].entity = nullptr;
    all.erase(std::remove(all.begin(), all.end(), this), all.end());
}

bool Entity::move(r::ivec2 target)
{
    if (target.x < 0 || target.y < 0 || target.x >= map.size().x || target.y >= map.size().y) return false;
    if (map[target].isSolid()) return false;
    map[position].entity = nullptr;
    position = target;
    map[position].entity = this;
    return true;
}
