#pragma once
#include "imgui.h"
#include "particle.h"

class FluidSimulatorWindow {
private:
    Particle particles[100];
public:
    FluidSimulatorWindow();
    void draw(ImGuiIO& io);
};