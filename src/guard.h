#pragma once

#include "entity.h"

/*
Main state:
* Normal (patrolling/idling)
* Alerted (go check out a specific location)
* Searching (something happened here, search the area)
* Hunting (we see/know where the enemy is, attack!)
*/
class Guard : public Entity
{
public:
    Guard(r::ivec2 position, int base_area_nr);
    void tick() override;
    void updateMapInfo() override;
    void draw(r::frontend::Renderer& renderer, r::ivec2 position);

    float view_direction = 0;
};
