#pragma once
#include "ComputeShader.h"
#include "Buffers.h"

class Simulation {
public:
    Simulation();

    void Simulate();

private:
    ComputeShader simulation_compute;
    StructuredBuffer position_buffer;
    StructuredBuffer predicted_position_buffer;
    StructuredBuffer velocity_buffer;
    StructuredBuffer density_buffer;
    StructuredBuffer spatial_indices;
    StructuredBuffer spatial_offsets;
};
