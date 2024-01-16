#include "fluidSimulatorWindow.h"
#include <time.h>
#include <iostream>
#include <imgui_internal.h>

struct HeatmapEntry {
    ImColor color;
    float percentage;
};

static std::vector<ImColor> constructHeatmap(size_t numberOfEntries, const std::vector<HeatmapEntry>& entries) {
    std::vector<ImColor> heatmap;

    if (entries.size() < 2) {
        return heatmap;
    }

    std::vector<HeatmapEntry> sortedEntries = entries;
    std::sort(sortedEntries.begin(), sortedEntries.end(),
        [](const HeatmapEntry& a, const HeatmapEntry& b) {
            return a.percentage < b.percentage;
        });

    for (size_t i = 0; i < numberOfEntries; ++i) {
        float t = static_cast<float>(i) / (numberOfEntries - 1);  // Interpolation factor

        size_t index = 0;
        while (index < sortedEntries.size() - 1 && t > sortedEntries[index + 1].percentage) {
            ++index;
        }

        const ImColor& color1 = sortedEntries[index].color;
        const ImColor& color2 = sortedEntries[index + 1].color;

        int r = static_cast<int>(ImLerp(color1.Value.x, color2.Value.x, (t - sortedEntries[index].percentage) / (sortedEntries[index + 1].percentage - sortedEntries[index].percentage)));
        int g = static_cast<int>(ImLerp(color1.Value.y, color2.Value.y, (t - sortedEntries[index].percentage) / (sortedEntries[index + 1].percentage - sortedEntries[index].percentage)));
        int b = static_cast<int>(ImLerp(color1.Value.z, color2.Value.z, (t - sortedEntries[index].percentage) / (sortedEntries[index + 1].percentage - sortedEntries[index].percentage)));
        int a = static_cast<int>(ImLerp(color1.Value.w, color2.Value.w, (t - sortedEntries[index].percentage) / (sortedEntries[index + 1].percentage - sortedEntries[index].percentage)));

        heatmap.push_back(ImColor(r, g, b, a));
    }

    return heatmap;
}

FluidSimulatorWindow::FluidSimulatorWindow()
{
    std::vector<HeatmapEntry> entries = {
        {ImColor(0, 0, 255), 0.0f},
        {ImColor(0, 255, 0), 0.25f},
        {ImColor(255, 255, 0), 0.50f},
        {ImColor(255, 0, 0), 1.0f}
    };

    heatmap = constructHeatmap(10, entries);
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

    ImGui::End();
}

void FluidSimulatorWindow::drawParticle(Float2 position, Float2 velocity) 
{
    auto radius = 8;
    float speed = Dot(velocity, velocity);
    speed = std::min(speed, 100.f);
    float percentage = speed / 100.f;
    size_t heatmapIndex = percentage * (heatmap.size() - 1);
    auto color = heatmap[heatmapIndex];
    ImU32 strokeColor = IM_COL32(255, 255, 255, 255);
    ImGui::GetWindowDrawList()->AddCircleFilled(position, radius, color);
    ImGui::GetWindowDrawList()->AddCircle(position, radius, strokeColor);
}
