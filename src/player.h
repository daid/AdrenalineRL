#pragma once

#include "entity.h"
#include <array>
#include <unordered_set>

class Item;
class Player : public Entity
{
public:
    Player(r::ivec2 position);
    virtual ~Player();
    void updateMapInfo() override;
    void draw(r::frontend::Renderer& renderer, r::ivec2 position);
    bool blocksPathFind() override { return false; }

    bool pickup(Item* item);
    bool dropItem(int index);
    bool equip(int index, int to_index);

    static constexpr int max_health = 5;
    int health = max_health;
    static constexpr int max_adrenaline = 5;
    int adrenaline = max_adrenaline;

    std::unordered_set<int> has_key;
    std::array<Item*, 10> items{};

    static Player* instance;
};
