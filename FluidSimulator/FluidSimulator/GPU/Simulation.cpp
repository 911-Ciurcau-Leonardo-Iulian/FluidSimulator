#include "Simulation.h"
#include "glad.h"
#include <iostream>
#include "../Vec2.h"

extern "C" {
    _declspec(dllexport) unsigned int NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#define SHADER_BASE_LOCATION "C:\\Users\\Andrei\\Documents\\Facultate\\Parallel and Distributed Programming\\FluidSimulator\\FluidSimulator\\FluidSimulator\\GPU\\Shaders\\"
#define SHADER_LOCATION(name) SHADER_BASE_LOCATION #name

#define PARTICLE_SIZE_X 0.005f
#define PARTICLE_SIZE_Y 0.005f

Simulation::Simulation()
{
    if (!gladLoadGL()) {
        std::cout << "Glad initialization failed";
        abort();
    }

    //simulation_compute = ComputeShader(SHADER_LOCATION(simulation_early.comp), 1, 1, 1);
    render_shader = Shader(SHADER_LOCATION(sprite.vert), SHADER_LOCATION(sprite.frag));

    Float2 square_vertices[6] = {
        { -1, -1 },
        { 1, -1 },
        { -1, 1 },
        { -1, 1 },
        { 1, -1 },
        { 1, 1 }
    };
    render_vertex_buffer = VertexBuffer(VertexDataType::Float2, 6, square_vertices);
    particle_count = 100;

    position_buffer = StructuredBuffer(sizeof(Float2), particle_count);
    predicted_position_buffer = StructuredBuffer(sizeof(Float2), particle_count);
    velocity_buffer = StructuredBuffer(sizeof(Float2), particle_count);
    density_buffer = StructuredBuffer(sizeof(Float2), particle_count);
    spatial_indices = StructuredBuffer(sizeof(unsigned int) * 3, particle_count);
    spatial_offsets = StructuredBuffer(sizeof(unsigned int), particle_count);
    SetInitialBufferData(particle_count);
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

void Simulation::SetInitialBufferData(size_t particle_count)
{
    const float center_x = 0.0f;
    const float center_y = 0.0f;

    size_t rows = sqrt(particle_count);
    size_t per_row_count = particle_count / rows;

    float row_x_start = center_x - ((float)per_row_count / 2) * PARTICLE_SIZE_X;
    float row_y = center_y - ((float)rows / 2) * PARTICLE_SIZE_Y;

    // At first, initialize the positions. The predicted positions will be the same as these positions
    std::vector<Float2> data;
    data.reserve(particle_count);
    for (size_t index = 0; index < rows; index++) {
        for (size_t column = 0; column < per_row_count; column++) {
            data.push_back({ row_x_start + (float)column * PARTICLE_SIZE_X, row_y });
        }
        row_y += PARTICLE_SIZE_Y;
    }
    for (size_t index = rows * per_row_count; index < particle_count; index++) {
        data.push_back({ row_x_start + (float)(index - rows * per_row_count) * PARTICLE_SIZE_X, row_y });
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

void Simulation::RenderParticles()
{
    render_shader.Use();
    render_vertex_buffer.Bind();
    position_buffer.Bind(0);
    velocity_buffer.Bind(1);
    render_vertex_buffer.Draw(1);
}
