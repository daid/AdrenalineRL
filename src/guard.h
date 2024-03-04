#pragma once

#include "entity.h"


class Guard : public Entity
{
public:
    Guard(r::ivec2 position);
    void tick() override;
    void updateMapInfo() override;
    void draw(r::frontend::Renderer& renderer, r::ivec2 position);

    float view_direction = 0;
};