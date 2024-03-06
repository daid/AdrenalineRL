#pragma once

#include "entity.h"

class Door : public Entity
{
public:
    Door(r::ivec2 position);
    void draw(r::frontend::Renderer& renderer, r::ivec2 position);
    bool blocksVision() { return true; }

    void tick() override;
    bool bump(Entity*) override;

    int open_counter = 0;

    Door* other = nullptr;
};

class KeyDoor : public Door
{
public:
    KeyDoor(r::ivec2 position, int area_nr);
    void draw(r::frontend::Renderer& renderer, r::ivec2 position);

    bool bump(Entity*) override;
    
    int area_nr;
};