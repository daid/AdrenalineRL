#pragma once

#include "r/vec.h"
#include "r/frontend/renderer.h"


class Entity
{
public:
    Entity(r::ivec2 position);
    virtual ~Entity();

    Entity(Entity&) = delete;
    Entity& operator=(Entity&) = delete;

    r::ivec2 pos() { return position; }
    bool move(r::ivec2 target);

    virtual void draw(r::frontend::Renderer& renderer, r::ivec2 position) { (void)renderer; (void)position; }
    virtual void tick() {}
    virtual void updateMapInfo() {}
    virtual bool bump() { return false; }

    virtual bool blocksVision() { return false; }

    static std::vector<Entity*> all;
private:
    r::ivec2 position;
};
