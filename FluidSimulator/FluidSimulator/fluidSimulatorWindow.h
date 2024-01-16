#pragma once
#include "imgui.h"
#include "Simulation.h"
#include <vector>

struct GLFWwindow;

class FluidSimulatorWindow {
public:
    Simulation simulation;
    std::vector<ImVec4> heatmap;

    FluidSimulatorWindow(
#if RUN_MPI
        int mpiWorkersCount
#endif
    );
    void draw(GLFWwindow* window, ImGuiIO& io);
    void drawParticle(Float2 position, Float2 velocity);
};
