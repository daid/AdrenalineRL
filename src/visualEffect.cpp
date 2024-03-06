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
    auto f = [&](int x, int y) {
        renderer.draw(position + offset + r::ivec2{ x, y}, {.2, .2, .2});
        renderer.draw(position + offset + r::ivec2{-x, y}, {.2, .2, .2});
        renderer.draw(position + offset + r::ivec2{ x,-y}, {.2, .2, .2});
        renderer.draw(position + offset + r::ivec2{-x,-y}, {.2, .2, .2});
        renderer.draw(position + offset + r::ivec2{ y, x}, {.2, .2, .2});
        renderer.draw(position + offset + r::ivec2{-y, x}, {.2, .2, .2});
        renderer.draw(position + offset + r::ivec2{ y,-x}, {.2, .2, .2});
        renderer.draw(position + offset + r::ivec2{-y,-x}, {.2, .2, .2});
    };

    int x = 0, y = radius;
    int d = 3 - 2 * radius; 
    f(x, y); 
    while (y >= x) {
        x++; 
        if (d > 0) { 
            y--;  
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6; 
        }
        f(x, y); 
    } 
}