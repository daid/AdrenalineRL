#pragma once

#include "entity.h"

class Player : public Entity
{
public:
    Player(r::ivec2 position);
    virtual ~Player();
    void updateMapInfo() override;
    void draw(r::frontend::Renderer& renderer, r::ivec2 position);

    static Player* instance;
};
