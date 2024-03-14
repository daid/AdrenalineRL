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
#include "item.h"
#include "roomTemplate.h"
#include "guard.h"
#include "player.h"
#include "visualEffect.h"
#include "shipGenerator.h"

static constexpr int sidebar_width = 15;

static void printItem(r::frontend::Renderer& renderer, r::ivec2 position, Item* item) {
    if (!item) return;
    item->draw(renderer, position);
    renderer.print(position + r::ivec2{1, 0}, {1, 1, 1}, item->name().c_str());
}

void tick() {
    for(auto ve : VisualEffect::all)
        delete ve;
    VisualEffect::all.clear();
    for(auto e : Entity::all)
        e->tick();
}


r::ActionQueue<int> queue;
r::Engine engine({.minimal_screen_size{60, 30}, .continuous_loop=false});

class DropItemMenu : public r::GameState
{
    void onRender(r::frontend::Renderer& renderer) override {
        auto size = r::ivec2{24, Player::instance->items.size() + 3};
        auto tl = (renderer.size() - size) / 2;
        renderer.drawBox(r::Recti{tl, size}, {1, 1, 1}, {0, 0, 0});
        renderer.print(tl + r::ivec2{2, 1}, {1, 1, 1}, "Drop which item:");
        renderer.draw(r::Recti{tl + r::ivec2{2, 2 + selection_index}, {size.x - 4, 1}}, ' ', {1, 1, 1}, {.2, .2, .7});
        for(size_t n=0; n<Player::instance->items.size(); n++) {
            printItem(renderer, tl + r::ivec2{3, 2 + n}, Player::instance->items[n]);
        }
    }

    void onKey(int key) override {
        switch(key) {
        case r::KEY_ESCAPE: engine.popState(); break;
        case r::KEY_UP: selection_index = (selection_index + Player::instance->items.size() - 1) % Player::instance->items.size(); break;
        case r::KEY_DOWN: selection_index = (selection_index + 1) % Player::instance->items.size(); break;
        case ' ':
        case r::KEY_RETURN:
            if (Player::instance->dropItem(selection_index)) {
                tick();
            }
            engine.popState();
            break;
        }
    }

    int selection_index = 0;
};

class EquipItemMenu : public r::GameState
{
    void onRender(r::frontend::Renderer& renderer) override {
        switch(state)
        {
        case State::SelectItem:{
            auto size = r::ivec2{24, Player::instance->items.size() + 1};
            auto tl = (renderer.size() - size) / 2;
            renderer.drawBox(r::Recti{tl, size}, {1, 1, 1}, {0, 0, 0});
            renderer.print(tl + r::ivec2{2, 1}, {1, 1, 1}, "Equip which item:");
            renderer.draw(r::Recti{tl + r::ivec2{2, 2 + selection_index}, {size.x - 4, 1}}, ' ', {1, 1, 1}, {.2, .2, .7});
            for(size_t n=0; n<Player::instance->items.size()-2; n++) {
                printItem(renderer, tl + r::ivec2{3, 2 + n}, Player::instance->items[n+2]);
            }
            }break;
        case State::SelectSlot:{
            auto size = r::ivec2{24, 5};
            auto tl = (renderer.size() - size) / 2;
            renderer.drawBox(r::Recti{tl, size}, {1, 1, 1}, {0, 0, 0});
            renderer.print(tl + r::ivec2{2, 1}, {1, 1, 1}, "In which slot:");
            renderer.draw(r::Recti{tl + r::ivec2{2, 2 + slot_index}, {size.x - 4, 1}}, ' ', {1, 1, 1}, {.2, .2, .7});
            for(size_t n=0; n<2; n++) {
                printItem(renderer, tl + r::ivec2{3, 2 + n}, Player::instance->items[n]);
            }
            }break;
        }
    }

    void onKey(int key) override {
        switch(key) {
        case r::KEY_ESCAPE: engine.popState(); break;
        case r::KEY_UP: switch(state) {
            case State::SelectItem: selection_index = (selection_index + Player::instance->items.size() - 2 - 1) % (Player::instance->items.size() - 2); break;
            case State::SelectSlot: slot_index ^= 1; break;
            }
            break;
        case r::KEY_DOWN: switch(state) {
            case State::SelectItem: selection_index = (selection_index + 1) % (Player::instance->items.size() - 2); break;
            case State::SelectSlot: slot_index ^= 1; break;
            }
            break;
        case ' ':
        case r::KEY_RETURN:
            switch(state)
            {
            case State::SelectItem:
                state = State::SelectSlot;
                if (!Player::instance->items[selection_index + 2])
                    engine.popState();
                break;
            case State::SelectSlot:
                if (Player::instance->equip(selection_index + 2, slot_index)) {
                    tick();
                }
                engine.popState();
                break;
            }
            break;
        }
    }

    enum class State {
        SelectItem, SelectSlot,
    } state = State::SelectItem;
    int selection_index = 0;
    int slot_index = 0;
};

class PickTarget : public r::GameState
{
public:
    PickTarget() {
        target_position = Player::instance->pos();
        r::VisitFOV(Player::instance->pos(), 20, [this](auto p) {
            if (p.x < 0 || p.y < 0 || p.x >= map.size().x || p.y >= map.size().y) return false;
            if (map[p].entity) {
                auto guard = dynamic_cast<Guard*>(map[p].entity);
                if (guard)
                    possible_targets.push_back(guard);
            }
            return !map[p].blocksVision();
        });
        std::sort(possible_targets.begin(), possible_targets.end(), [](const Entity* a, const Entity* b) {
            return r::length(r::vec2(Player::instance->pos() - a->pos())) < r::length(r::vec2(Player::instance->pos() - b->pos()));
        });
        if (!possible_targets.empty())
            target_position = possible_targets.front()->pos();
        for(auto e : possible_targets)
            if (e->id() == last_target_id)
                target_position = e->pos();
    }

    void onRender(r::frontend::Renderer& renderer) override {
        auto camera_offset = renderer.size() / 2 - Player::instance->pos();
        camera_offset.x -= sidebar_width / 2;

        for(auto p : r::TraceLine(Player::instance->pos(), target_position)) {
            renderer.draw(p + camera_offset, {.3, .3, .0});
        }
        renderer.draw(target_position + camera_offset, 'X', {1, 1, .0});
    }

    void onKey(int key) override {
        switch(key) {
        case r::KEY_ESCAPE: engine.popState(); break;
        case r::KEY_LEFT: target_position.x -= 1; break;
        case r::KEY_RIGHT: target_position.x += 1; break;
        case r::KEY_UP: target_position.y -= 1; break;
        case r::KEY_DOWN: target_position.y += 1; break;
        case '\t':{
            int current_index = -1;
            for(size_t n=0; n<possible_targets.size(); n++) {
                if (possible_targets[n]->pos() == target_position)
                    current_index = n;
            }
            if (!possible_targets.empty())
                target_position = possible_targets[(current_index+1)%possible_targets.size()]->pos();
            }break;
        case ' ':
        case r::KEY_RETURN:
            last_target_id = 0;
            for(auto e : possible_targets)
                if (e->pos() == target_position)
                    last_target_id = e->id();
            engine.popState();
            break;
        }
    }

    std::vector<Entity*> possible_targets;
    r::ivec2 target_position;
    static inline size_t last_target_id;
};

class MainState : public r::GameState
{
public:
    MainState() {
    }

    void onUpdate(float delta) override { (void)delta; }

    void onRender(r::frontend::Renderer& renderer) override {
        for(auto& tile : map) {
            tile.enemy_sight = Tile::EnemySight::None;
            tile.enemy_sight_level = Tile::EnemySightLevel::None;
            tile.player_visible = false;
        }
        for(auto e : Entity::all) {
            e->updateMapInfo();
        }

        auto camera_offset = renderer.size() / 2 - Player::instance->pos();
        camera_offset.x -= sidebar_width / 2;
        renderer.draw(r::Recti{{}, renderer.size()}, ' ', {1, 1, 1}, {0.0,0.0,0.0});
        for(auto p : r::Recti{{}, map.size()}) {
            r::Color bg_color = {0.05, 0.05, 0.05};
            if (map[p].player_visible) bg_color = {0.1, 0.1, 0.1};
            if (map[p].enemy_sight == Tile::EnemySight::Minor) {
                switch(map[p].enemy_sight_level) {
                    case Tile::EnemySightLevel::None: bg_color = {0.0, 0.2, 0.0}; break;
                    case Tile::EnemySightLevel::Alert: bg_color = {0.2, 0.2, 0.0}; break;
                    case Tile::EnemySightLevel::Hunt: bg_color = {0.2, 0.0, 0.0}; break;
                }
            }
            if (map[p].enemy_sight == Tile::EnemySight::Major) {
                switch(map[p].enemy_sight_level) {
                    case Tile::EnemySightLevel::None: bg_color = {0.0, 0.3, 0.0}; break;
                    case Tile::EnemySightLevel::Alert: bg_color = {0.3, 0.3, 0.0}; break;
                    case Tile::EnemySightLevel::Hunt: bg_color = {0.3, 0.0, 0.0}; break;
                }
            }
            switch(map[p].type) {
            case Tile::Type::Void:
                renderer.draw(p + camera_offset, ' ', {0.3, 0.3, 0.3}, bg_color);
                break;
            case Tile::Type::Floor:
                renderer.draw(p + camera_offset, '.', {0.4, 0.3, 0.3}, bg_color);
                break;
            case Tile::Type::Wall:
                renderer.draw(p + camera_offset, '#', {0.3, 0.3, 0.4}, bg_color);
                break;
            }
            if (!map[p].items.empty())
                map[p].items.back()->draw(renderer, p + camera_offset);
        }
        for(auto e : Entity::all)
            e->drawLower(renderer, camera_offset);
        for(auto ve : VisualEffect::all)
            ve->draw(renderer, camera_offset);
        for(auto p : r::Recti{{}, map.size()}) {
            if (map[p].entity)
                map[p].entity->draw(renderer, p + camera_offset);
            else if (map[p].second_entity)
                map[p].second_entity->draw(renderer, p + camera_offset);
        }

        drawUI(renderer);
    }

    void drawUI(r::frontend::Renderer& renderer) {
        auto rs = renderer.size();
        auto tl = r::ivec2{rs.x - sidebar_width, 0};
        renderer.draw(r::Recti{tl, {sidebar_width, rs.y}}, ' ', {1, 1, 1}, {0.0,0.0,0.0});
        renderer.draw(r::Recti{tl, {1, rs.y}}, '|', {1, 1, 1}, {0.0,0.0,0.0});
        renderer.print(tl + r::ivec2{2, 1}, {1, 1, 1}, "Health");
        renderer.draw(tl + r::ivec2{2, 2}, '[', {1, 1, 1});
        renderer.draw(tl + r::ivec2{3 + Player::max_health, 2}, ']', {1, 1, 1});
        renderer.print(tl + r::ivec2{2, 4}, {1, 1, 1}, "Adrenaline");
        renderer.draw(tl + r::ivec2{2, 5}, '[', {1, 1, 1});
        renderer.draw(tl + r::ivec2{3 + Player::max_adrenaline, 5}, ']', {1, 1, 1});
        renderer.print(tl + r::ivec2{2, 7}, {1, 1, 1}, "Equiped");
        renderer.print(tl + r::ivec2{2, 8}, {1, 1, 1}, "1:");
        renderer.print(tl + r::ivec2{2, 9}, {1, 1, 1}, "2:");
        renderer.print(tl + r::ivec2{2, 10}, {1, 1, 1}, "Inventory");

        if (Player::instance) {
            r::Color health_color = {.3, 1, .3};
            if (Player::instance->health < Player::max_health / 2)
                health_color = {1, .1, .1};
            for(int n=0; n<Player::instance->health; n++)
                renderer.draw(tl + r::ivec2{3 + n, 2}, '#', health_color);

            for(int n=0; n<Player::instance->adrenaline; n++)
                renderer.draw(tl + r::ivec2{3 + n, 5}, '#', {1, .3, .3});
            
            printItem(renderer, tl + r::ivec2{4, 8}, Player::instance->items[0]);
            printItem(renderer, tl + r::ivec2{4, 9}, Player::instance->items[1]);
            for(size_t n=2; n<Player::instance->items.size(); n++) {
                printItem(renderer, tl + r::ivec2{3, 9 + n}, Player::instance->items[n]);
            }

            auto& tile = map[Player::instance->pos()];
            if (!tile.items.empty()) {
                renderer.print(tl + r::ivec2{2, 20}, {1, 1, 1}, "Floor");
                for(size_t n=0; n<tile.items.size(); n++) {
                    printItem(renderer, tl + r::ivec2{3, 21 + n}, tile.items[n]);
                }
            }
        }
    }
    
    void onKey(int key) override {
        switch(key) {
        case r::KEY_ESCAPE: engine.popState(); break;
        case r::KEY_LEFT: tryPlayerMove(r::ivec2{-1, 0}); break;
        case r::KEY_RIGHT: tryPlayerMove(r::ivec2{1, 0}); break;
        case r::KEY_UP: tryPlayerMove(r::ivec2{0, -1}); break;
        case r::KEY_DOWN: tryPlayerMove(r::ivec2{0, 1}); break;
        case 'g': case 'p': tryPickup(); break;
        case 'd': engine.pushState<DropItemMenu>(); break;
        case 'e': engine.pushState<EquipItemMenu>(); break;
        case '1': tryUseItem(0); break;
        case '2': tryUseItem(1); break;
        case ' ': tick(); break;
        }
    }

    void tryPlayerMove(r::ivec2 offset) {
        if (!Player::instance) return;
        auto target = Player::instance->pos() + offset;
        if (map[target].entity && map[target].entity->bump(Player::instance)) {
            tick();
            return;
        }
        if (Player::instance->move(target))
            tick();
    }

    void tryPickup() {
        if (!Player::instance) return;
        auto& tile = map[Player::instance->pos()];
        if (tile.items.empty()) return;
        if (!Player::instance->pickup(tile.items[0])) return;
        tile.items.erase(tile.items.begin());
        tick();
    }

    void tryUseItem(int index) {
        if (!Player::instance->items[index]) return;
        engine.pushState<PickTarget>();
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

    new Player({4, 4 + 7 * 10});
    Player::instance->pickup(new Item());
    Player::instance->items[9] = new Item();
    map[Player::instance->pos()].items.push_back(new Item());
    map[Player::instance->pos()].items[0]->type = Item::Type::Key;
    map[Player::instance->pos()].items[0]->subtype = 1;

    engine.pushState<MainState>();
    engine.run();
    return 0;
}
