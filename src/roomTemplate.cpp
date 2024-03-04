#include "roomTemplate.h"
#include <string.h>
#include <stdio.h>


static constexpr const char* room_template_input = R"EOS(

###..###
#......#
#......#
........
........
#......#
#......#
###..###

########
########
########
........
........
########
########
########

########
#......#
#......#
........
........
#......#
#......#
########

########
########
########
.....###
.....###
########
########
########

########
#......#
#......#
.......#
.......#
#......#
#......#
########

########
########
########
###.....
###.....
###..###
###..###
###..###

########
#......#
#......#
#.......
#.......
#......#
#......#
###..###

########
########
########
........
........
###..###
###..###
###..###

########
#......#
#......#
........
........
#......#
#......#
###..###

########
########
########
###..###
###..###
########
########
########

)EOS";

std::unordered_map<int, std::vector<RoomTemplate>> RoomTemplate::templates;


static void addTemplate(RoomTemplate& t)
{
    int connection_idx = 0;
    auto s = t.tiles.size();
    if (t.tiles[{s.x / 2, 0}] == Tile::Type::Floor) connection_idx |= 1 << 0;
    if (t.tiles[{s.x - 1, s.y / 2}] == Tile::Type::Floor) connection_idx |= 1 << 1;
    if (t.tiles[{s.x / 2, s.y - 1}] == Tile::Type::Floor) connection_idx |= 1 << 2;
    if (t.tiles[{0, s.y / 2}] == Tile::Type::Floor) connection_idx |= 1 << 3;
    for(const auto& temp : RoomTemplate::templates[connection_idx])
        if (temp.tiles == t.tiles)
            return;
    RoomTemplate::templates[connection_idx].push_back(t);
}

void RoomTemplate::init()
{
    auto end_ptr = room_template_input - 2;
    while(true) {
        auto start_ptr = end_ptr + 2;
        end_ptr = strstr(start_ptr, "\n\n");
        if (!end_ptr) return;
        int line_count = 1;
        int line_length = 0;
        int max_line_length = 0;
        for(auto c = start_ptr; c != end_ptr; c++) {
            if (*c == '\n') {
                line_count += 1;
                line_length = 0;
            } else {
                line_length += 1;
                max_line_length = std::max(max_line_length, line_length);
            }
        }
        if (max_line_length < 1) continue;

        RoomTemplate t;
        t.tiles.resize({max_line_length, line_count}, Tile::Type::Void);
        r::vec2 pos{0, 0};
        for(auto c = start_ptr; c != end_ptr; c++) {
            switch(*c) {
            case '\n': pos.y += 1; pos.x = -1; break;
            case '.': t.tiles[pos] = Tile::Type::Floor; break;
            case '#': t.tiles[pos] = Tile::Type::Wall; break;
            }
            pos.x += 1;
        }
        addTemplate(t);
        for(int n=0; n<3; n++) {
            t = t.rotate();
            addTemplate(t);
        }
    }
}

RoomTemplate RoomTemplate::rotate()
{
    RoomTemplate result;
    result.tiles.resize({tiles.size().y, tiles.size().x});
    auto h = tiles.size().y - 1;
    for(auto p : r::Recti{{}, tiles.size()}) {
        result.tiles[r::ivec2{h-p.y, p.x}] = tiles[p];
    }
    return result;
}
