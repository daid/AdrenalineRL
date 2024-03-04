#include "player.h"
#include "map.h"
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

void Player::updateMapInfo()
{
    r::VisitFOV(pos(), 20, [this](auto p) {
        if (p.x < 0 || p.y < 0 || p.x >= map.size().x || p.y >= map.size().y) return false;
        map[p].player_visible = true;
        return !map[p].blocksVision();
    });
}