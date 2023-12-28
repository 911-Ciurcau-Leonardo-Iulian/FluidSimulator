#include "fluidSimulatorWindow.h"
#include <time.h>

FluidSimulatorWindow::FluidSimulatorWindow()
{
    srand(time(NULL));

    for (auto& particle : FluidSimulatorWindow::particles) 
    {
        particle = Particle({(float) (rand() % 400 + 400), (float) (rand() % 300)});
    }
}

void FluidSimulatorWindow::draw(ImGuiIO& io)
{
    ImGui::Begin("Fluid Simulator Main Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar); 

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

    ImVec2 rectPos(50, ImGui::GetFontSize() * 10);
    ImVec2 rectSize(ImGui::GetWindowWidth() - rectPos.x * 2, ImGui::GetWindowHeight() - rectPos.y - ImGui::GetFontSize());
    ImU32 rectColor = IM_COL32(155, 155, 155, 255);

    ImGui::GetWindowDrawList()->AddRect(rectPos, ImVec2(rectPos.x + rectSize.x, rectPos.y + rectSize.y), rectColor);

    ImGuiStyle& style = ImGui::GetStyle();
    ImU32 bgColor = ImGui::GetColorU32(style.Colors[ImGuiCol_WindowBg]);
    ImGui::GetWindowDrawList()->AddLine(rectPos, {rectPos.x + rectSize.x, rectPos.y}, bgColor);

    for (auto& particle : FluidSimulatorWindow::particles) 
    {
        particle.draw();
    }

    ImGui::End();
}
