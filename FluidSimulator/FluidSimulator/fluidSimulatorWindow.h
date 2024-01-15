#pragma once
#include "imgui.h"
#include "Simulation.h"

class FluidSimulatorWindow {
private:
    Simulation simulation;
public:
    FluidSimulatorWindow();
    void draw(ImGuiIO& io);
    static void drawParticle(Float2 position);
};