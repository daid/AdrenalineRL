#pragma once

#include "entity.h"
#include "r/direction.h"

/*
Main state:
* Normal (patrolling/idling)
* Alerted (go check out a specific location)
* Searching (something happened here, search the area)
* Hunting (we see/know where the enemy is, attack!)
*/
class Guard : public Entity
{
public:
    static constexpr float view_range = 20;
    static constexpr float short_range = 12;
    Guard(r::ivec2 position, int base_area_nr);
    void tick() override;
    void normalAction();
    void alertedAction();
    void huntingAction();
    void updateMapInfo() override;
    void drawLower(r::frontend::Renderer& renderer, r::ivec2 offset) override;
    void draw(r::frontend::Renderer& renderer, r::ivec2 position) override;
    bool pathFindTo(r::ivec2 target);
    void seePlayer(bool far_away);

    int base_area_nr;

    float view_direction = 0;

    enum class BaseState {
        Normal,
        Alerted,
        Searching,
        Hunting,
    } base_state = BaseState::Normal;
    int normal_action_delay = 0;
    enum class NormalState {
        Idle,
        Patrol,
    } normal_state = NormalState::Idle;
    enum class AlertedState {
        MoveToTarget,
    } alerted_state = AlertedState::MoveToTarget;
    int hunting_counter = 0;
    int hunting_delay = 0;
    r::ivec2 target_position;

    struct PathNode {
        r::ivec2 position;
        r::Direction direction;

        bool operator==(const PathNode& p) const { return position == p.position && direction == p.direction; }
    };
    std::vector<PathNode> path;
    std::optional<r::ivec2> shot_fired_at;
};
