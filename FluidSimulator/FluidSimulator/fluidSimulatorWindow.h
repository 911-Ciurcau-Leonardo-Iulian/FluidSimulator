#pragma once
#include "imgui.h"
#include "particle.h"
#include "GPU/Simulation.h"

class FluidSimulatorWindow {
public:
    FluidSimulatorWindow();

    void draw(ImGuiIO& io);

private:
    Simulation simulation{ 10 };
};
