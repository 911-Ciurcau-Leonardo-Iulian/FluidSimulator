#include "fluidSimulatorWindow.h"
#include <time.h>
#include <iostream>

FluidSimulatorWindow::FluidSimulatorWindow()
{
    srand(time(NULL));
    simulation.Start();
}

void FluidSimulatorWindow::draw(GLFWwindow* window, ImGuiIO& io)
{
    simulation.Update(window, io.DeltaTime);

    ImGui::Begin("Fluid Simulator Main Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar); 

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

    ImVec2 rectPos(50, ImGui::GetFontSize() * 10);
    ImVec2 rectSize(ImGui::GetWindowWidth() - rectPos.x * 2, ImGui::GetWindowHeight() - rectPos.y - ImGui::GetFontSize());
    ImU32 rectColor = IM_COL32(155, 155, 155, 255);

    ImGui::GetWindowDrawList()->AddRect(rectPos, ImVec2(rectPos.x + rectSize.x, rectPos.y + rectSize.y), rectColor);

    ImGuiStyle& style = ImGui::GetStyle();
    ImU32 bgColor = ImGui::GetColorU32(style.Colors[ImGuiCol_WindowBg]);
    ImGui::GetWindowDrawList()->AddLine(rectPos, {rectPos.x + rectSize.x, rectPos.y}, bgColor);

    for (auto& particle : simulation.physics.Positions) 
    {
        drawParticle(particle);
        //std::cout << '(' << particle.x << ',' << particle.y << ")\n";
    }
    //std::cout << "\n\n";

    ImGui::End();
}

void FluidSimulatorWindow::drawParticle(Float2 position) 
{
    auto radius = 10;
    auto color = IM_COL32(155, 155, 0, 255);
    ImU32 strokeColor = IM_COL32(255, 255, 255, 255);
    ImGui::GetWindowDrawList()->AddCircleFilled(position, radius, color);
    ImGui::GetWindowDrawList()->AddCircle(position, radius, strokeColor);
}
