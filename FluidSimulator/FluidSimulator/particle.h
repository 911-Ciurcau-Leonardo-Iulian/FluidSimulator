#pragma once
#include "imgui.h"
#include <algorithm>


class Particle {
private:
    static float radius;

public:
    ImVec2 position;
    ImVec2 velocity;
    ImVec2 acceleration;
    static const ImU32 color = IM_COL32(155, 155, 0, 255);
    static const ImU32 strokeColor = IM_COL32(255, 255, 255, 255);

    Particle() {}
    Particle(const ImVec2& position, const ImVec2& velocity={0, 0}, const ImVec2& acceleration={0, 0});
    void draw();
};