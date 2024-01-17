#include "GPUSort.h"
#include "ShaderLocation.h"
#include <cmath>

struct Settings {
    unsigned int num_entries;
    unsigned int group_width;
    unsigned int group_height;
    unsigned int step_index;
};

static size_t NextPowerOfTwo(size_t value) {
    size_t power = 1;
    while (power < value) {
        power <<= 1;
    }
    return power;
}

void GPUSort::Initialize()
{
    sort_compute = ComputeShader(SHADER_LOCATION(sort.comp), 128, 1, 1);
    sort_compute.CreateUniformBlock("Settings", sizeof(Settings));
    offsets_compute = ComputeShader(SHADER_LOCATION(sort_calculate_offsets.comp), 128, 1, 1);
}

void GPUSort::Execute(StructuredBuffer spatial_indices_buffer, StructuredBuffer offset_buffer, size_t entry_count) {
    // Launch each step of the sorting algorithm (once the previous step is complete)
    // Number of steps = [log2(n) * (log2(n) + 1)] / 2
    // where n = nearest power of 2 that is greater or equal to the number of inputs
    int stage_count = (int)std::log2f(NextPowerOfTwo(entry_count));

    sort_compute.Bind();
    spatial_indices_buffer.Bind(0);
    for (int stage_index = 0; stage_index < stage_count; stage_index++)
    {
        for (int step_index = 0; step_index < stage_index + 1; step_index++)
        {
            // Calculate some pattern stuff
            int group_width = 1 << (stage_index - step_index);
            int group_height = 2 * group_width - 1;
            Settings settings;
            settings.group_width = group_width;
            settings.group_height = group_height;
            settings.num_entries = entry_count;
            settings.step_index = step_index;
            sort_compute.SetUniformBlock("Settings", &settings);
            sort_compute.BindUniformBlock(0);
            // Run the sorting step on the GPU
            sort_compute.Dispatch(NextPowerOfTwo(entry_count) / 2, 1, 1);
        }
    }

    // Now the offset calculation part comes
    offset_buffer.Bind(1);
    offsets_compute.BindAndDispatch(entry_count, 1, 1, false);
}
