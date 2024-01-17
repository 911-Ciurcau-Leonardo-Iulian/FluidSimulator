#include "Simulation.h"
#include "glad.h"
#include <iostream>
#include "../Vec2.h"
#include "ShaderLocation.h"
#include "GeneralSettings.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <GLFW\glfw3.h>

extern "C" {
    _declspec(dllexport) unsigned int NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#define PARTICLE_SIZE 0.005f

static void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    std::cout << "Source: ";
    switch (source) {
    case GL_DEBUG_SOURCE_API:
        std::cout << "API | ";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        std::cout << "APPLICATION | ";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        std::cout << "OTHER | ";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cout << "SHADER COMPILER | ";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        std::cout << "THIRD PARTY | ";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cout << "WINDOW SYSTEM | ";
        break;
    }

    std::cout << "Type: ";
    switch (type) {
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "DEPRECATED | ";
        break;
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "ERROR | ";
        break;
    case GL_DEBUG_TYPE_MARKER:
        std::cout << "TYPE MARKER | ";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "OTHER | ";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "PERFORMANCE | ";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        std::cout << "POP GROUP | ";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "PORTABILITY | ";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        std::cout << "PUSH GROUP | ";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "UNDEFINED BEHAVIOUR | ";
        break;
    }

    std::cout << " Severity: ";
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cout << "HIGH | ";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << "LOW | ";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cout << "MEDIUM | ";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cout << "NOTIFICATION | ";
        break;
    }

    std::cout << "Message: " << message << "\n";
}

void Simulation::ChangeParticleCount(size_t _particle_count)
{
    particle_count = _particle_count;
    position_buffer.SetNewDataSize(sizeof(Float2), particle_count);
    predicted_position_buffer.SetNewDataSize(sizeof(Float2), particle_count);
    velocity_buffer.SetNewDataSize(sizeof(Float2), particle_count);
    density_buffer.SetNewDataSize(sizeof(Float2), particle_count);
    spatial_indices.SetNewDataSize(sizeof(unsigned int) * 3, particle_count);
    spatial_offsets.SetNewDataSize(sizeof(unsigned int), particle_count);
    SetInitialBufferData(particle_count);
}

void Simulation::DoFrame(Float2 normalized_mouse_pos, bool is_left_mouse_pressed, bool is_right_mouse_pressed, float delta_time)
{
    delta_time = std::min(0.005f, delta_time);
    SetFrameParameters(normalized_mouse_pos, is_left_mouse_pressed, is_right_mouse_pressed, delta_time);
    FrameCompute();
}

void Simulation::Initialize()
{
    glDebugMessageCallback(DebugCallback, nullptr);

    glDisable(GL_CULL_FACE);

    // Use the simulation early compute to hold the general settings
    simulation_early_compute = ComputeShader(SHADER_LOCATION(simulation_early.comp), 128, 1, 1);
    calculate_density_compute = ComputeShader(SHADER_LOCATION(calculate_density.comp), 128, 1, 1);
    calculate_pressure_compute = ComputeShader(SHADER_LOCATION(calculate_pressure.comp), 128, 1, 1);
    calculate_viscosity_update_pos_compute = ComputeShader(SHADER_LOCATION(calculate_viscosity_update_pos.comp), 128, 1, 1);

    simulation_early_compute.CreateUniformBlock("Settings", sizeof(GeneralSettings));

    render_shader = Shader(SHADER_LOCATION(sprite.vert), SHADER_LOCATION(sprite.frag));

    render_vertex_buffer = VertexBuffer(DataType::Float2, 6);
    circle_alpha_texture = CreateCircleAlphaTexture(256, 256, 1.0f);
    std::vector<HeatmapEntry> entries = {
        { Float4{28 / 255.0f, 70 / 255.0f, 158 / 255.0f, 1.0f}, 0.15f },
        { Float4{94 / 255.0f, 190 / 255.0f, 149 / 255.0f, 1.0f}, 0.5f},
        { Float4{200 / 255.0f, 200 / 255.0f, 17 / 255.0f, 1.0f}, 0.70f},
        { Float4{200 / 255.0f, 73 / 255.0f, 43 / 255.0f, 1.0f}, 1.0f}
    };
    std::vector<UChar4> heatmap_values = ConstructHeatmap(1024, entries);
    heatmap_texture.SetData(DataType::UNorm4, 1024, heatmap_values.data());
    circle_alpha_texture.Bind(0);
    heatmap_texture.Bind(1);
    particle_count = 50'000;
    SetAspectRatio(2500.0f / 1200.0f);

    position_buffer = StructuredBuffer(sizeof(Float2), particle_count);
    predicted_position_buffer = StructuredBuffer(sizeof(Float2), particle_count);
    velocity_buffer = StructuredBuffer(sizeof(Float2), particle_count);
    density_buffer = StructuredBuffer(sizeof(Float2), particle_count);
    spatial_indices = StructuredBuffer(sizeof(unsigned int) * 3, particle_count);
    spatial_offsets = StructuredBuffer(sizeof(unsigned int), particle_count);
    SetInitialBufferData(particle_count);
    SetInitialSettingsData();

    gpu_sort.Initialize();
}

void Simulation::SetInitialSettingsData()
{
    const float FACTOR = 2.0f;

    GeneralSettings* settings = (GeneralSettings*)simulation_early_compute.GetUniformBlockData("Settings");
    settings->collision_damping = 0.4f;
    settings->gravity = 150.0f * FACTOR;
    settings->interaction_input_radius = 100.0f;
    settings->interaction_input_strength = 0.0f;
    settings->near_pressure_multiplier = 1.75f * FACTOR;
    settings->pressure_multiplier = 22.5f * FACTOR;
    settings->num_particles = particle_count;
    settings->smoothing_radius = 7.5f * FACTOR;
    settings->target_density = 150.0f * FACTOR;
    settings->viscosity_strength = 1.75f * FACTOR;
    mouse_click_strength = 50000.0f;
    simulation_early_compute.SetUniformBlockDirty("Settings");
}

void Simulation::FrameCompute()
{
    // Early dispatch
    GeneralSettings* general_settings = (GeneralSettings*)simulation_early_compute.GetUniformBlockData("Settings");
    simulation_early_compute.BindUniformBlock(0);
    position_buffer.Bind(0);
    velocity_buffer.Bind(1);
    predicted_position_buffer.Bind(2);
    spatial_offsets.Bind(3);
    spatial_indices.Bind(4);
    simulation_early_compute.BindAndDispatch(particle_count, 1, 1, false);

    // GPU spatial sorting
    gpu_sort.Execute(spatial_indices, spatial_offsets, particle_count);

    // We need to rebing the uniform block with the general settings for the rest of the pipeline
    simulation_early_compute.BindUniformBlock(0);
    // The density dispatch
    density_buffer.Bind(0);
    predicted_position_buffer.Bind(1);
    spatial_offsets.Bind(2);
    spatial_indices.Bind(3);
    calculate_density_compute.BindAndDispatch(particle_count, 1, 1);

    // The bindings for the pressure include those from the density
    velocity_buffer.Bind(4);
    calculate_pressure_compute.BindAndDispatch(particle_count, 1, 1);

    // The bindings for the final dispatch include those from the pressure dispatch
    position_buffer.Bind(5);
    calculate_viscosity_update_pos_compute.BindAndDispatch(particle_count, 1, 1);
}

void Simulation::SetInitialBufferData(size_t particle_count)
{
    const float center_x = 0.0f;
    const float center_y = 0.0f;

    size_t rows = sqrt(particle_count);
    size_t per_row_count = particle_count / rows;

    float row_x_start = (center_x - ((float)per_row_count / 2) * PARTICLE_SIZE) * 2000.0f;
    float row_y = (center_y - ((float)rows / 2) * PARTICLE_SIZE * aspect_ratio) * 2000.0f;

    // At first, initialize the positions. The predicted positions will be the same as these positions
    std::vector<Float2> data;
    data.reserve(particle_count);
    for (size_t index = 0; index < rows; index++) {
        for (size_t column = 0; column < per_row_count; column++) {
            data.push_back({ row_x_start + (float)column * PARTICLE_SIZE * 2000.0f, row_y });
        }
        row_y += PARTICLE_SIZE * aspect_ratio * 2000.0f;
    }
    for (size_t index = rows * per_row_count; index < particle_count; index++) {
        data.push_back({ row_x_start + (float)(index - rows * per_row_count) * PARTICLE_SIZE * 2000.0f, row_y });
    }
    position_buffer.SetNewData(sizeof(Float2), particle_count, data.data());
    predicted_position_buffer.SetNewData(sizeof(Float2), particle_count, data.data());
    
    // Set the initial velocities to 0.0f
    // We can reuse the buffer from the positions
    for (size_t index = 0; index < particle_count; index++) {
        data[index] = { 0.0f };
    }
    velocity_buffer.SetNewData(sizeof(Float2), particle_count, data.data());
}

void Simulation::SetFrameParameters(Float2 normalized_mouse_pos, bool is_left_mouse_pressed, bool is_right_mouse_pressed, float delta_time)
{
    GeneralSettings* settings = (GeneralSettings*)simulation_early_compute.GetUniformBlockData("Settings");
    float smoothing_radius = settings->smoothing_radius;
    settings->poly6_scaling_factor = 4 / (M_PI * pow(smoothing_radius, 8));
    settings->spiky_pow3_scaling_factor = 10 / (M_PI * pow(smoothing_radius, 5));
    settings->spiky_pow2_scaling_factor = 6 / (M_PI * pow(smoothing_radius, 4));
    settings->spiky_pow3_derivative_scaling_factor = 30 / (pow(smoothing_radius, 5) * M_PI);
    settings->spiky_pow2_derivative_scaling_factor = 12 / (pow(smoothing_radius, 4) * M_PI);
    settings->delta_time = delta_time;
    settings->num_particles = particle_count;

    float interaction_strength = 0;
    if (is_left_mouse_pressed || is_right_mouse_pressed)
    {
        interaction_strength = is_left_mouse_pressed ? -mouse_click_strength : mouse_click_strength;
    }

    settings->interaction_input_point = normalized_mouse_pos * 2000.0f;
    // This must be inverted because of OpenGL order;
    settings->interaction_input_point.y = -settings->interaction_input_point.y;
    settings->interaction_input_strength = interaction_strength;

    simulation_early_compute.SetUniformBlockDirty("Settings");
}

void Simulation::RenderParticles()
{
    render_shader.Use();
    render_shader.SetFloat("scale", PARTICLE_SIZE * 0.45f);
    render_shader.SetFloat("aspect_ratio", aspect_ratio);
    render_shader.SetFloat("max_speed", 2000.0f);
    render_shader.SetTexture("circle_alpha", 0);
    render_shader.SetTexture("Heatmap", 1);
    render_vertex_buffer.Bind();
    position_buffer.Bind(0);
    velocity_buffer.Bind(1);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    render_vertex_buffer.Draw(particle_count);
}
