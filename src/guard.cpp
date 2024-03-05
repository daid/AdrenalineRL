#include "guard.h"
#include "map.h"
#include "player.h"
#include "shipGenerator.h"
#include "keyDoor.h"
#include "r/fov.h"
#include "r/line.h"
#include "r/random.h"
#include "r/direction.h"
#include "r/astar.h"


static float angleDiff(float a1, float angle) {
    angle = a1 - angle;
    while(angle > M_PI)
        angle -= M_PI * 2;
    while(angle < -M_PI)
        angle += M_PI * 2;
    return angle;
}

static float angleDiff(r::vec2 vector, float angle) {
    return angleDiff(atan2(vector.y, vector.x), angle);
}

static r::Direction angleToDirection(float angle) {
    angle = angleDiff(0, angle);
    if (std::abs(angle) < M_PI * .25)
        return r::Direction::Right;
    if (std::abs(angle) > M_PI * .75)
        return r::Direction::Left;
    if (angle > 0)
        return r::Direction::Up;
    return r::Direction::Down;
}

static r::ivec2 randomPositionInArea(int area_nr) {
    auto& ai = area_info[area_nr];
    auto p = ai.tiles[r::irandom(0, ai.tiles.size() - 1)];
    return p * 7 + r::ivec2{r::irandom(3, 4), r::irandom(3, 4)};
}


Guard::Guard(r::ivec2 position, int base_area_nr) : Entity(position), base_area_nr(base_area_nr)
{
}

void Guard::tick()
{
    shot_fired_at.reset();
    if (Player::instance) {
        r::VisitFOV(pos(), view_range, [this](auto p) {
            if (p.x < 0 || p.y < 0 || p.x >= map.size().x || p.y >= map.size().y) return false;
            r::vec2 diff = p - pos();
            if ((diff.x != 0 || diff.y != 0) && std::abs(angleDiff(diff, view_direction)) <= M_PI * 0.26f) {
                if (p == Player::instance->pos()) {
                    seePlayer(r::length(diff) >= short_range);
                }
            }
            return !map[p].blocksVision();
        });
    }

    switch(base_state)
    {
    case BaseState::Normal: normalAction(); break;
    case BaseState::Alerted: alertedAction(); break;
    case BaseState::Searching: break;
    case BaseState::Hunting: huntingAction(); break;
    }

    //view_direction += M_PI * .125;
    //move(pos() + r::ivec2{1, 0});
}

void Guard::normalAction()
{
    if (normal_action_delay) {
        normal_action_delay--;
        return;
    }
    switch(normal_state) {
    case NormalState::Idle:
        target_position = randomPositionInArea(base_area_nr);
        normal_state = NormalState::Patrol;
        normal_action_delay = 1;
        break;
    case NormalState::Patrol:
        //normal_action_delay = 1;
        if (!pathFindTo(target_position))
            normal_state = NormalState::Idle;
        break;
    }
}

void Guard::alertedAction()
{
    switch(alerted_state) {
    case AlertedState::MoveToTarget:
        if (!pathFindTo(target_position)) {
            normal_state = NormalState::Idle;
            base_state = BaseState::Normal;
        }
        break;
    }
}

void Guard::huntingAction()
{
    if (hunting_counter++ > 10) {
        base_state = BaseState::Searching;
        return;
    }
    if (hunting_delay) {
        hunting_delay--;
        return;
    }
    if (Player::instance) {
        target_position = Player::instance->pos();
        bool can_hit = true;
        if (std::abs(angleDiff(target_position - pos(), view_direction)) > M_PI * 0.26f)
            can_hit = false;
        for(auto p : r::TraceLine(pos(), target_position)) {
            if (map[p].isSolid() && p != target_position && p != pos()) {
                can_hit = false;
                break;
            }
        }
        if (can_hit) {
            shot_fired_at = target_position;
            hunting_delay = 1;
        } else {
            pathFindTo(target_position);
        }
    }
}

void Guard::seePlayer(bool far_away)
{
    switch(base_state)
    {
    case BaseState::Normal:
    case BaseState::Alerted:
    case BaseState::Searching:
        if (far_away) {
            base_state = BaseState::Alerted;
            alerted_state = AlertedState::MoveToTarget;
            target_position = Player::instance->pos();
            path.clear();
        } else {
            hunting_counter = 0;
            hunting_delay = 0;
            base_state = BaseState::Hunting;
            target_position = Player::instance->pos();
            path.clear();
        }
        break;
    case BaseState::Hunting:
        hunting_counter = 0;
        break;
    }
}

namespace std { template <> struct hash<Guard::PathNode> {
    size_t operator()(const Guard::PathNode& p) const {
      return hash<r::ivec2>()(p.position) ^ (static_cast<int>(p.direction) << 8);
    }
  };
}

bool Guard::pathFindTo(r::ivec2 target)
{
    path = r::AStar<PathNode>({pos(), angleToDirection(view_direction)}, {target, static_cast<r::Direction>(5)}, [](PathNode p) {
        std::vector<std::pair<PathNode, float>> v;
        if (p.direction == static_cast<r::Direction>(5)) return v;
        for(auto d : r::DirectionIterator()) {
            auto t = p.position + offset(d);
            if (map[t].isWalkable()) {
                float score = (d == p.direction) ? 1.0 : 2.0;
                v.push_back({{t, d}, score});
            }
        }
        v.push_back({{p.position, static_cast<r::Direction>(5)}, 0.1});
        return v;
    }, [](PathNode a, PathNode b) -> float { return r::length(r::vec2(b.position - a.position)); });
    if (path.size() == 0 || path[0].position == pos()) {
        path.clear();
        return false;
    }
    float view_target = 0;
    auto diff = path[0].position - pos();
    if (diff.x > 0) view_target = 0;
    if (diff.x < 0) view_target = M_PI;
    if (diff.y > 0) view_target = M_PI * .5;
    if (diff.y < 0) view_target = M_PI * 1.5;
    
    auto ad = angleDiff(view_direction, view_target);
    auto rotation_per_turn = M_PI * .125;
    if (base_state == BaseState::Hunting) rotation_per_turn *= 2.0;
    if (std::abs(ad) < rotation_per_turn + 0.005) {
        view_direction = view_target;
        if (!move(path[0].position)) {
            auto door = dynamic_cast<Door*>(map[path[0].position].entity);
            if (door)
                door->bump(this);
        }
    } else {
        if (ad > 0) view_direction -= rotation_per_turn;
        else view_direction += rotation_per_turn;
    }
    return true;
}

void Guard::drawLower(r::frontend::Renderer& renderer, r::ivec2 offset)
{
    for(auto p : path) {
        if (p.direction == r::Direction::Left || p.direction == r::Direction::Right)
            renderer.draw(p.position + offset, '-', {.5, .5, .5});
        else if (p.direction == r::Direction::Up || p.direction == r::Direction::Down)
            renderer.draw(p.position + offset, '|', {.5, .5, .5});
    }
    if (shot_fired_at.has_value()) {
        for(auto p : r::TraceLine(pos(), shot_fired_at.value())) {
            renderer.draw(p + offset, '*', {.9, .0, .0});
        }
    }
}

void Guard::draw(r::frontend::Renderer& renderer, r::ivec2 position)
{
    renderer.draw(position, 'G', {1, 1, 1});
}

void Guard::updateMapInfo()
{
    r::VisitFOV(pos(), view_range, [this](auto p) {
        if (p.x < 0 || p.y < 0 || p.x >= map.size().x || p.y >= map.size().y) return false;
        r::vec2 diff = p - pos();
        if ((diff.x != 0 || diff.y != 0) && std::abs(angleDiff(diff, view_direction)) <= M_PI * 0.26f) {
            if (r::length(diff) < short_range)
                map[p].enemy_sight = Tile::EnemySight::Major;
            else if (map[p].enemy_sight == Tile::EnemySight::None)
                map[p].enemy_sight = Tile::EnemySight::Minor;
            if (base_state == BaseState::Hunting) {
                map[p].enemy_sight_level = Tile::EnemySightLevel::Hunt;
            } else if (base_state != BaseState::Normal && map[p].enemy_sight_level == Tile::EnemySightLevel::None) {
                map[p].enemy_sight_level = Tile::EnemySightLevel::Alert;
            }
        }
        return !map[p].blocksVision();
    });
}
