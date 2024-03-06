#include "keyDoor.h"
#include "map.h"
#include "player.h"
#include "r/direction.h"
#include <stdio.h>


Door::Door(r::ivec2 position) : Entity(position)
{
    for(auto d : r::DirectionIterator()) {
        Door* o = dynamic_cast<Door*>(map[pos() + r::offset(d)].entity);
        if (o) {
            other = o;
            o->other = this;
        }
    }
}

void Door::draw(r::frontend::Renderer& renderer, r::ivec2 position)
{
    r::Color color = {0.3, 0.8, 0.0};
    if (open_counter)
        color *= .5;
    renderer.draw(position, '#', color);
}

void Door::tick()
{
    if (open_counter) {
        open_counter--;
        if (map[pos()].entity) open_counter = 5;
        if (other && map[other->pos()].entity) open_counter = 5;
        if (open_counter == 0) {
            map[pos()].entity = this;
            map[pos()].second_entity = nullptr;
            if (other) {
                other->open_counter = 0;
                map[other->pos()].entity = other;
                map[other->pos()].second_entity = nullptr;
            }
        }
    }
}

bool Door::bump(Entity* bumper)
{
    (void)bumper;
    map[pos()].entity = nullptr;
    map[pos()].second_entity = this;
    open_counter = 5;
    if (other) {
        other->open_counter = 5;
        map[other->pos()].entity = nullptr;
        map[other->pos()].second_entity = other;
    }
    return true;
}

KeyDoor::KeyDoor(r::ivec2 position, int area_nr) : Door(position), area_nr(area_nr)
{
}

bool KeyDoor::bump(Entity* bumper)
{
    if (bumper == Player::instance) {
        if (Player::instance->has_key.find(area_nr) == Player::instance->has_key.end())
            return false;
    }
    return Door::bump(bumper);
}

void KeyDoor::draw(r::frontend::Renderer& renderer, r::ivec2 position)
{
    r::Color color = {1, 0.3, 0};
    if (Player::instance && Player::instance->has_key.find(area_nr) != Player::instance->has_key.end())
        color = {0.3, 0.8, 0};
    if (open_counter)
        color *= .5;
    renderer.draw(position, '0' + area_nr, color);
}
