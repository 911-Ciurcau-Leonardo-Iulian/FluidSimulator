#include "fluidSimulatorWindow.h"
#include <time.h>
#include "GPU\glad.h"
#include <GLFW\glfw3.h>
#include <iostream>

FluidSimulatorWindow::FluidSimulatorWindow()
{
    srand(time(NULL));
    if (!gladLoadGL()) {
        std::cout << "Glad initialization failed";
        abort();
    }
    simulation.Initialize();
}

void FluidSimulatorWindow::Draw(bool is_left_mouse_pressed, bool is_right_mouse_pressed, ImGuiIO& io)
{
    Float2 mouse_pos = { ImGui::GetMousePos().x , ImGui::GetMousePos().y };
    Float2 normalized_mouse_pos = { mouse_pos.x / width * 2.0f - 1.0f, mouse_pos.y / height * 2.0f - 1.0f };
    simulation.DoFrame(normalized_mouse_pos, is_left_mouse_pressed, is_right_mouse_pressed, io.DeltaTime);
    simulation.RenderParticles();
}

void FluidSimulatorWindow::SetWindowDimensions(size_t _width, size_t _height)
{
    width = _width;
    height = _height;
    float aspect_ratio = (float)width / (float)height;
    simulation.SetAspectRatio(aspect_ratio);
}
