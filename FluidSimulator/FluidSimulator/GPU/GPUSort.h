#pragma once
#include "ComputeShader.h"
#include "Buffers.h"

class GPUSort {
public:
    void Initialize();

    void Execute(StructuredBuffer spatial_indices_buffer, StructuredBuffer offset_buffer, size_t entry_count);

private:
    ComputeShader sort_compute;
    ComputeShader offsets_compute;
};
