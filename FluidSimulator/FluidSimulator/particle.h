#pragma once
#include "imgui.h"

class Particle {
public:
    ImVec2 position;
    float radius;
    ImVec2 velocity;
    ImVec2 acceleration;
    static const ImU32 color = IM_COL32(0, 0, 155, 255);

    Particle(const ImVec2& position, const float& radius, const ImVec2& velocity, const ImVec2& acceleration);
    void draw();
};