#include "shipGenerator.h"
#include "roomTemplate.h"
#include "guard.h"
#include "keyDoor.h"
#include "player.h"
#include "map.h"
#include "r/random.h"
#include <stdio.h>


r::Vector2<ShipTile> ship_tiles;
std::vector<AreaInfo> area_info;


static void mazeFill(int area_nr)
{
    std::vector<r::ivec2> cells;
    for(auto p : r::Recti({}, ship_tiles.size()))
        if (ship_tiles[p].area_nr == area_nr)
            cells.push_back(p);
    std::vector<std::pair<r::ivec2, int>> walls;
    auto start = cells[r::irandom(0, cells.size()-1)];
    //printf("start: %d %d\n", start.x, start.y);
    for(int n=0; n<4; n++)
        walls.push_back({start, n});
    int done_count = 0, fail_count = 0;
    while(!walls.empty()) {
        auto idx = r::irandom(0, walls.size() - 1);
        auto [cell, direction] = walls[idx];
        walls.erase(walls.begin() + idx);
        auto target = cell;
        switch(direction) {
        case 0: if (cell.y > 0) target = cell + r::ivec2{ 0,-1}; break;
        case 1: if (cell.x < ship_tiles.size().x - 1) target = cell + r::ivec2{ 1, 0}; break;
        case 2: if (cell.y < ship_tiles.size().y - 1) target = cell + r::ivec2{ 0, 1}; break;
        case 3: if (cell.x > 0) target = cell + r::ivec2{-1, 0}; break;
        }
        if (cell != target && ship_tiles[target].area_nr == area_nr && ship_tiles[target].connection_mask == 0) {
            //printf("Dig: %d,%d %d %d,%d\n", cell.x, cell.y, direction, target.x, target.y);
            ship_tiles[cell].connection_mask |= (1 << direction);
            ship_tiles[target].connection_mask |= (1 << ((direction + 2) % 4));
            for(int n=0; n<4; n++)
                walls.push_back({target, n});
            done_count += 1;
        } else {
            fail_count += 1;
        }
    }
    //printf("%d %d\n", done_count, fail_count);

    for(auto p : r::Recti({}, ship_tiles.size() - r::ivec2{1, 1})) {
        if (ship_tiles[p].area_nr == area_nr && ship_tiles[p + r::ivec2{1, 0}].area_nr == area_nr && r::irandom(0, 100) < 5) {
            ship_tiles[p].connection_mask |= (1 << 1);
            ship_tiles[p + r::ivec2{1, 0}].connection_mask |= (1 << 3);
        }
        if (ship_tiles[p].area_nr == area_nr && ship_tiles[p + r::ivec2{0, 1}].area_nr == area_nr && r::irandom(0, 100) < 5) {
            ship_tiles[p].connection_mask |= (1 << 2);
            ship_tiles[p + r::ivec2{0, 1}].connection_mask |= (1 << 0);
        }
    }
}

static bool addArea(r::ivec2 start, int area_nr)
{
    r::ivec2 size = {3, 3};
    auto rng = r::irandom(0, 100);
    if (rng < 30) size = {3, 2};
    if (rng < 60) size = {2, 3};
    if (rng < 65) size = {4, 3};
    if (rng < 70) size = {3, 4};
    start.x -= r::irandom(0, size.x - 1);
    start.y -= r::irandom(0, size.y - 1);
    if (start.x < 0 || start.y < 0 || start.x + size.x >= ship_tiles.size().x || start.y + size.y >= ship_tiles.size().y) return false;
    for(auto p : r::Recti(start, size))
        if (ship_tiles[p].area_nr != -1)
            return false;
    for(auto p : r::Recti(start, size))
        ship_tiles[p].area_nr = area_nr;
    mazeFill(area_nr);
    return true;
}

static void generateShipTileAreas(int area_count)
{
    ship_tiles.resize({20, 20});
    map.resize(ship_tiles.size() * 7 + r::ivec2{1, 1});
    area_info.clear();
    area_info.resize(area_count);

    ship_tiles[{0, 10}].area_nr = -2;
    ship_tiles[{0, 10}].connection_mask = 2;
    while(!addArea({1, 10}, 0));
    for(int area_nr=1; area_nr<area_count; area_nr++) {
        std::vector<r::ivec2> start_options;
        for(auto p : r::Recti({}, ship_tiles.size())) {
            if (ship_tiles[p].area_nr != -1) {
                if (p.x > 0 && ship_tiles[p + r::ivec2{-1, 0}].area_nr == -1) start_options.push_back(p + r::ivec2{-1, 0});
                if (p.y > 0 && ship_tiles[p + r::ivec2{ 0,-1}].area_nr == -1) start_options.push_back(p + r::ivec2{ 0,-1});
                if (p.x < ship_tiles.size().x - 1 && ship_tiles[p + r::ivec2{ 1, 0}].area_nr == -1) start_options.push_back(p + r::ivec2{ 1, 0});
                if (p.y < ship_tiles.size().y - 1 && ship_tiles[p + r::ivec2{ 0, 1}].area_nr == -1) start_options.push_back(p + r::ivec2{ 0, 1});
            }
        }
        while(!addArea(start_options[r::irandom(0, start_options.size() - 1)], area_nr));
    }
    new Door(r::ivec2{1, 10} * 7 + r::ivec2{0, 3});
    new Door(r::ivec2{1, 10} * 7 + r::ivec2{0, 4});
    ship_tiles[{1, 10}].connection_mask |= 8;

    for(auto p : r::Recti({}, ship_tiles.size())) {
        if (ship_tiles[p].area_nr >= 0)
            area_info[ship_tiles[p].area_nr].tiles.push_back(p);
    }

    //Add doors between areas
    for(int source_area=0; source_area<area_count; source_area++) {
        for(int target_area=source_area+1; target_area<area_count; target_area++) {
            std::vector<std::pair<r::ivec2, int>> options;
            for(auto p : r::Recti({}, ship_tiles.size() - r::ivec2{1, 1})) {
                if (ship_tiles[p].area_nr == source_area && ship_tiles[p + r::ivec2{1, 0}].area_nr == target_area)
                    options.push_back({p, 0});
                if (ship_tiles[p].area_nr == target_area && ship_tiles[p + r::ivec2{1, 0}].area_nr == source_area)
                    options.push_back({p, 0});
                if (ship_tiles[p].area_nr == source_area && ship_tiles[p + r::ivec2{0, 1}].area_nr == target_area)
                    options.push_back({p, 1});
                if (ship_tiles[p].area_nr == target_area && ship_tiles[p + r::ivec2{0, 1}].area_nr == source_area)
                    options.push_back({p, 1});
            }
            if (options.size() > 0) {
                auto [p, dir] = options[r::irandom(0, options.size() - 1)];
                if (dir == 0) {
                    ship_tiles[p].connection_mask |= 1 << 1;
                    ship_tiles[p + r::ivec2{1, 0}].connection_mask |= 1 << 3;
                    new KeyDoor(p * 7 + r::ivec2{7, 3}, target_area);
                    new KeyDoor(p * 7 + r::ivec2{7, 4}, target_area);
                } else {
                    ship_tiles[p].connection_mask |= 1 << 2;
                    ship_tiles[p + r::ivec2{0, 1}].connection_mask |= 1 << 0;
                    new KeyDoor(p * 7 + r::ivec2{3, 7}, target_area);
                    new KeyDoor(p * 7 + r::ivec2{4, 7}, target_area);
                }
            }
        }
    }
}

void generateShip()
{
    generateShipTileAreas(5);

    for(auto p : r::Recti({}, ship_tiles.size())) {
        //if (p.x == 0) printf("\n"); printf("%2d", ship_tiles[p].area_nr);
        if (ship_tiles[p].area_nr != -1) {
            const auto& tlist = RoomTemplate::templates[ship_tiles[p].connection_mask];
            const auto& t = tlist[r::irandom(0, tlist.size() - 1)];
            auto offset = p * 7;
            for(auto p : r::Recti({}, t.tiles.size()))
                map[p + offset].type = t.tiles[p];
        }
    }

    for(int area_nr=0; area_nr<area_info.size(); area_nr++) {
        auto& ai = area_info[area_nr];
        auto p = ai.tiles[r::irandom(0, ai.tiles.size() - 1)];
        auto target = p * 7 + r::ivec2{r::irandom(3, 4), r::irandom(3, 4)};
        new Guard(target, area_nr);
    }
}
