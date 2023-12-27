#include "fluidSimulatorWindow.h"

void FluidSimulatorWindow::draw(ImGuiIO& io)
{
    ImGui::Begin("Simulation", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);  
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
}