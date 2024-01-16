#pragma once
#include "ComputeShader.h"
#include "Buffers.h"
#include "Shader.h"
#include "VertexBuffer.h"

class Simulation {
public:
    Simulation();

    void ChangeParticleCount(size_t particle_count);

    void Simulate();

private:
    void SetInitialBufferData(size_t particle_count);

    void RenderParticles();

    Shader render_shader;
    // This contains the sprite square vertices
    VertexBuffer render_vertex_buffer;

    ComputeShader simulation_compute;
    StructuredBuffer position_buffer;
    StructuredBuffer predicted_position_buffer;
    StructuredBuffer velocity_buffer;
    StructuredBuffer density_buffer;
    StructuredBuffer spatial_indices;
    StructuredBuffer spatial_offsets;

    size_t particle_count;
};
