#pragma once

#include "r/frontend/renderer.h"

class VisualEffect
{
public:
    VisualEffect();
    virtual ~VisualEffect() = default;

    virtual void draw(r::frontend::Renderer& renderer, r::ivec2 offset) = 0;

    static std::vector<VisualEffect*> all;
};

class LineEffect : public VisualEffect
{
public:
    LineEffect(r::ivec2 start, r::ivec2 end) : start(start), end(end) {}

    void draw(r::frontend::Renderer& renderer, r::ivec2 offset) override;

    r::ivec2 start;
    r::ivec2 end;
};

class NoiseEffect : public VisualEffect
{
public:
    NoiseEffect(r::ivec2 position, float radius) : position(position), radius(radius) {}

    void draw(r::frontend::Renderer& renderer, r::ivec2 offset) override;

    r::ivec2 position;
    float radius;
};