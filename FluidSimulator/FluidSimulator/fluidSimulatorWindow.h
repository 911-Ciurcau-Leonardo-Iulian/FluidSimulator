#pragma once
#include "imgui.h"
#include "particle.h"
#include "GPU/Simulation.h"

class FluidSimulatorWindow {
public:
    FluidSimulatorWindow();

    void Draw(bool is_left_mouse_pressed, bool is_right_mouse_pressed, ImGuiIO& io);

    void SetWindowDimensions(size_t width, size_t height);

//private:
    Simulation simulation;
    size_t width;
    size_t height;
};
