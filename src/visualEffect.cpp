#include "visualEffect.h"
#include "r/line.h"

std::vector<VisualEffect*> VisualEffect::all;

VisualEffect::VisualEffect()
{
    all.push_back(this);
}

void LineEffect::draw(r::frontend::Renderer& renderer, r::ivec2 offset)
{
    auto prev = start;
    for(auto p : r::TraceLine(start, end)) {
        char c = '*';
        if (p.x != prev.x && p.y != prev.y) {
            c = ((p.x > prev.x && p.y > prev.y) || (p.x < prev.x && p.y < prev.y)) ? '\\' : '/';
        } else if (p.x != prev.x) {
            c = '-';
        } else if (p.y != prev.y) {
            c = '|';
        }
        renderer.draw(prev + offset, c, {.9, .0, .0});
        prev = p;
    }
}

void NoiseEffect::draw(r::frontend::Renderer& renderer, r::ivec2 offset)
{
    for(int y=-radius; y<=radius; y++) {
        int sx = std::sqrt(radius*radius - y*y);
        renderer.draw(position + offset + r::ivec2{-sx, y}, {.2, .2, .2});
        renderer.draw(position + offset + r::ivec2{sx, y}, {.2, .2, .2});
    }
}