#pragma once

#include "entity.h"

class Player : public Entity
{
public:
    Player(r::ivec2 position);
    virtual ~Player();
    void updateMapInfo() override;
    void draw(r::frontend::Renderer& renderer, r::ivec2 position);

    static constexpr int max_health = 5;
    int health = max_health;

    static Player* instance;
};
