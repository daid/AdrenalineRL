#include "player.h"
#include "map.h"
#include "item.h"
#include "r/fov.h"


Player* Player::instance;

Player::Player(r::ivec2 position) : Entity(position)
{
    instance = this;
}

Player::~Player()
{
    instance = nullptr;
}

void Player::draw(r::frontend::Renderer& renderer, r::ivec2 position)
{
    renderer.draw(position, '@', {1, 1, 1});
}

bool Player::pickup(Item* item)
{
    if (item->type == Item::Type::Key) {
        has_key.insert(item->subtype);
        delete item;
        return true;
    }
    for(size_t n=0; n<items.size(); n++) {
        if (!items[n]) {
            items[n] = item;
            return true;
        }
    }
    return false;
}

bool Player::dropItem(int index)
{
    if (!items[index]) return false;
    map[pos()].items.push_back(items[index]);
    items[index] = nullptr;
    return true;
}

bool Player::equip(int index, int to_index)
{
    if (!items[index]) return false;
    std::swap(items[index], items[to_index]);
    return true;
}

void Player::updateMapInfo()
{
    r::VisitFOV(pos(), 20, [this](auto p) {
        if (p.x < 0 || p.y < 0 || p.x >= map.size().x || p.y >= map.size().y) return false;
        map[p].player_visible = true;
        return !map[p].blocksVision();
    });
}