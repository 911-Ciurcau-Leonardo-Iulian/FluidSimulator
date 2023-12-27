#pragma once
#include "imgui.h"

class FluidSimulatorWindow {
private:
    FluidSimulatorWindow() {}
public:
    static void draw(ImGuiIO& io);
};