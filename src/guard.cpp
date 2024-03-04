#include "guard.h"
#include "map.h"
#include "r/fov.h"

static float angleDiff(r::vec2 vector, float angle) {
    angle = atan2(vector.y, vector.x) - angle;
    while(angle > M_PI)
        angle -= M_PI * 2;
    while(angle < -M_PI)
        angle += M_PI * 2;
    return angle;
}


Guard::Guard(r::ivec2 position) : Entity(position)
{
}

void Guard::tick()
{
    view_direction += M_PI * .125;
    //move(pos() + r::ivec2{1, 0});
}

void Guard::draw(r::frontend::Renderer& renderer, r::ivec2 position)
{
    renderer.draw(position, 'G', {1, 1, 1});
}

void Guard::updateMapInfo()
{
    r::VisitFOV(pos(), 20, [this](auto p) {
        if (p.x < 0 || p.y < 0 || p.x >= map.size().x || p.y >= map.size().y) return false;
        r::vec2 diff = p - pos();
        if ((diff.x != 0 || diff.y != 0) && std::abs(angleDiff(diff, view_direction)) <= M_PI * 0.26f) {
            if (r::length(diff) < 10)
                map[p].enemy_sight = Tile::EnemySight::Major;
            else if (map[p].enemy_sight == Tile::EnemySight::None)
                map[p].enemy_sight = Tile::EnemySight::Minor;
        }
        return !map[p].blocksVision();
    });
}