#include "keyDoor.h"
#include "map.h"
#include <stdio.h>


Door::Door(r::ivec2 position) : Entity(position)
{
}

void Door::draw(r::frontend::Renderer& renderer, r::ivec2 position)
{
    if (open_counter)
        renderer.draw(position, '#', {0.5, 0.15, 0});
    else
        renderer.draw(position, '#', {1, 0.3, 0});
}

void Door::tick()
{
    if (open_counter) {
        open_counter--;
        if (map[pos()].entity) open_counter = 5;
        if (open_counter == 0) {
            map[pos()].entity = this;
            map[pos()].second_entity = nullptr;
        }
    }
}

bool Door::bump()
{
    map[pos()].entity = nullptr;
    map[pos()].second_entity = this;
    open_counter = 5;
    return true;
}

KeyDoor::KeyDoor(r::ivec2 position, int area_nr) : Door(position), area_nr(area_nr)
{
}

void KeyDoor::draw(r::frontend::Renderer& renderer, r::ivec2 position)
{
    if (open_counter)
        renderer.draw(position, '0' + area_nr, {0.5, 0.15, 0});
    else
        renderer.draw(position, '0' + area_nr, {1, 0.3, 0});
}
