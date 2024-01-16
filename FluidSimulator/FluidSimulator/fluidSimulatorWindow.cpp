#define IMGUI_DEFINE_MATH_OPERATORS
#include "fluidSimulatorWindow.h"
#include <time.h>
#include <iostream>
#include <imgui_internal.h>

struct HeatmapEntry {
    ImColor color;
    float percentage;
};

static std::vector<ImVec4> constructHeatmap(size_t numberOfEntries, const std::vector<HeatmapEntry>& entries) {
    std::vector<ImVec4> heatmap;
    heatmap.resize(numberOfEntries);

    auto fill_range = [&heatmap, numberOfEntries](ImVec4 first_color, ImVec4 second_color, float first_percentage, float second_percentage) {
        size_t range_start = floor(first_percentage * numberOfEntries);
        size_t range_end = floor(second_percentage * numberOfEntries);

        float step = 1.0f / (range_end - range_start);
        for (size_t index = 0; index < range_end - range_start; index++) {
            float interpolation_factor = index * step;
            float one_minus_factor = 1.0f - interpolation_factor;
            heatmap[range_start + index] = first_color * ImVec4(interpolation_factor, interpolation_factor, interpolation_factor, interpolation_factor) +
                second_color * ImVec4(one_minus_factor, one_minus_factor, one_minus_factor, one_minus_factor);
        }
    };

    if (entries[0].percentage > 0.0f) {
        fill_range(entries[0].color, entries[0].color, 0.0f, entries[0].percentage);
    }

    for (size_t index = 0; index < entries.size() - 1; index++) {
        fill_range(entries[index].color, entries[index + 1].color, entries[index].percentage, entries[index + 1].percentage);
    }

    if (entries[entries.size() - 1].percentage < 1.0f) {
        ImColor last_color = entries[entries.size() - 1].color;
        fill_range(last_color, last_color, entries[entries.size() - 1].percentage, 1.0f);
    }

    return heatmap;
}

FluidSimulatorWindow::FluidSimulatorWindow()
{
    std::vector<HeatmapEntry> entries = {
        {ImColor(28, 70, 158), 0.15f},
        {ImColor(94, 255, 149), 0.5f},
        {ImColor(251, 248, 17), 0.70f},
        {ImColor(173, 73, 43), 1.0f}
    };
    /*std::vector<HeatmapEntry> entries = {
        {ImColor(200, 53, 43), 0.2f},
        {ImColor(251, 248, 17), 0.80f}
    };*/

    heatmap = constructHeatmap(1024, entries);
    srand(time(NULL));
    simulation.Start();
}

void FluidSimulatorWindow::draw(GLFWwindow* window, ImGuiIO& io)
{
    simulation.Update(window, io.DeltaTime);
    float FACTOR = 5;

    ImGui::Begin("Fluid Simulator Main Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus); 

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    
    /*ImVec2 rectPos(50, ImGui::GetFontSize() * 10);
    ImVec2 rectSize(ImGui::GetWindowWidth() - rectPos.x * 2, ImGui::GetWindowHeight() - rectPos.y - ImGui::GetFontSize());
    ImU32 rectColor = IM_COL32(155, 155, 155, 255);

    ImGui::GetWindowDrawList()->AddRect(rectPos, ImVec2(rectPos.x + rectSize.x, rectPos.y + rectSize.y), rectColor);

    ImGuiStyle& style = ImGui::GetStyle();
    ImU32 bgColor = ImGui::GetColorU32(style.Colors[ImGuiCol_WindowBg]);
    ImGui::GetWindowDrawList()->AddLine(rectPos, {rectPos.x + rectSize.x, rectPos.y}, bgColor);*/
    for (int i = 0; i < simulation.physics.Positions.size(); i++) {
        drawParticle(simulation.physics.Positions[i], simulation.physics.Velocities[i]);
    }

    ImGui::GetWindowDrawList()->AddCircle(simulation.physics.interactionInputPoint, simulation.physics.interactionInputRadius, IM_COL32(255, 30, 30, 255));

    ImGui::End();
}

void FluidSimulatorWindow::drawParticle(Float2 position, Float2 velocity) 
{
    auto radius = 4;
    float speed = Dot(velocity, velocity);
    static float MAX_SPEED = 0.0f;
    MAX_SPEED = std::max(MAX_SPEED, speed);

    speed = std::min(speed, 300'000.f);
    float percentage = speed / 300'000.f;
    size_t heatmapIndex = percentage * (heatmap.size() - 1);
    auto color = heatmap[heatmapIndex];
    ImU32 strokeColor = IM_COL32(255, 255, 255, 255);
    ImGui::GetWindowDrawList()->AddCircleFilled(position, radius, ImColor(color));
    //ImGui::GetWindowDrawList()->AddCircle(position, radius, strokeColor);
}
