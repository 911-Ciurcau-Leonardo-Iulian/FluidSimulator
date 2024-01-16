#pragma once
#include "imgui.h"
#include "Simulation.h"

struct GLFWwindow;

class FluidSimulatorWindow {
private:
    Simulation simulation;
public:
    FluidSimulatorWindow();
    void draw(GLFWwindow* window, ImGuiIO& io);
    static void drawParticle(Float2 position);
};
