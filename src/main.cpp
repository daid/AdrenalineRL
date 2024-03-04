#include "r/engine.h"
#include "r/keys.h"
#include "r/actionqueue.h"
#include "r/vector2.h"
#include "r/random.h"
#include "r/line.h"
#include "r/fov.h"
#include "r/ui/root.h"
#include "r/ui/panel.h"
#include "r/ui/label.h"
#include "r/ui/vertical.h"
#include <stdio.h>
#include "map.h"
#include "roomTemplate.h"
#include "guard.h"
#include "player.h"
#include "shipGenerator.h"


r::ActionQueue<int> queue;
r::Engine engine({.minimal_screen_size{40, 30}});


class MainState : public r::GameState
{
public:
    MainState() {
    }

    void onUpdate(float delta) override { (void)delta; }

    void onRender(r::frontend::Renderer& renderer) override {
        for(auto& tile : map) {
            tile.enemy_sight = Tile::EnemySight::None;
            tile.player_visible = false;
        }
        for(auto e : Entity::all) {
            e->updateMapInfo();
        }

        auto camera_offset = renderer.size() / 2 - Player::instance->pos();
        renderer.draw(r::Recti{{}, renderer.size()}, ' ', {1, 1, 1}, {0.0,0.0,0.0});
        for(auto p : r::Recti{{}, map.size()}) {
            r::Color bg_color = {0.05, 0.05, 0.05};
            if (map[p].player_visible) bg_color = {0.1, 0.1, 0.1};
            if (map[p].enemy_sight == Tile::EnemySight::Minor) bg_color = {0.0, 0.2, 0.0};
            if (map[p].enemy_sight == Tile::EnemySight::Major) bg_color = {0.0, 0.3, 0.0};
            switch(map[p].type) {
            case Tile::Type::Void:
                renderer.draw(p + camera_offset, ' ', {0.3, 0.3, 0.3}, bg_color);
                break;
            case Tile::Type::Floor:
                renderer.draw(p + camera_offset, '.', {1.0, 0.3, 0.3}, bg_color);
                break;
            case Tile::Type::Wall:
                renderer.draw(p + camera_offset, '#', {0.3, 0.3, 0.3}, bg_color);
                break;
            }
            if (map[p].entity)
                map[p].entity->draw(renderer, p + camera_offset);
            else if (map[p].second_entity)
                map[p].second_entity->draw(renderer, p + camera_offset);
        }
    }
    
    void onKey(int key) override {
        switch(key) {
        case r::KEY_ESCAPE: engine.popState(); break;
        case r::KEY_LEFT: tryPlayerMove(r::ivec2{-1, 0}); break;
        case r::KEY_RIGHT: tryPlayerMove(r::ivec2{1, 0}); break;
        case r::KEY_UP: tryPlayerMove(r::ivec2{0, -1}); break;
        case r::KEY_DOWN: tryPlayerMove(r::ivec2{0, 1}); break;
        case ' ': tick(); break;
        }
    }

    void tryPlayerMove(r::ivec2 offset) {
        auto target = Player::instance->pos() + offset;
        if (map[target].entity && map[target].entity->bump()) {
            tick();
            return;
        }
        if (Player::instance->move(target))
            tick();
    }

    void tick() {
        for(auto e : Entity::all) {
            e->tick();
        }
    }

    void onMouseDown(r::ivec2 p, int button) {
        (void)p;
        (void)button;
    }
};

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    RoomTemplate::init();

    generateShip();

    //new Guard({15, 15});
    //new Guard({5, 5});
    new Player({4, 4 + 7 * 10});

    engine.pushState<MainState>();
    engine.run();
    return 0;
}
