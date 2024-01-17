#pragma once
#include "ComputeShader.h"
#include "Buffers.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include "Texture.h"
#include "GPUSort.h"
#include "GeneralSettings.h"

class Simulation {
public:
    void ChangeParticleCount(size_t particle_count);

    void DoFrame(Float2 normalized_mouse_pos, bool is_left_mouse_pressed, bool is_right_mouse_pressed, float delta_time);

    inline GeneralSettings* GetGeneralSettings() {
        return (GeneralSettings*)simulation_early_compute.GetUniformBlockData("Settings");
    }

    float* GetMouseClickStrength() {
        return &mouse_click_strength;
    }

    void Initialize();

    void RenderParticles();

    inline void Reset() {
        ChangeParticleCount(particle_count);
    }

    inline void SetAspectRatio(float new_aspect_ratio) {
        aspect_ratio = new_aspect_ratio;
    }

    void SetInitialSettingsData();

    void SetFrameParameters(Float2 normalized_mouse_pos, bool is_left_mouse_pressed, bool is_right_mouse_pressed, float delta_time);

private:
    void FrameCompute();

    void SetInitialBufferData(size_t particle_count);

    Shader render_shader;
    // This contains the sprite square vertices
    VertexBuffer render_vertex_buffer;
    Texture2D circle_alpha_texture;
    Texture1D heatmap_texture;

    ComputeShader simulation_early_compute;
    ComputeShader calculate_density_compute;
    ComputeShader calculate_pressure_compute;
    ComputeShader calculate_viscosity_update_pos_compute;

    StructuredBuffer position_buffer;
    StructuredBuffer predicted_position_buffer;
    StructuredBuffer velocity_buffer;
    StructuredBuffer density_buffer;
    StructuredBuffer spatial_indices;
    StructuredBuffer spatial_offsets;

    GPUSort gpu_sort;

    size_t particle_count;
    float aspect_ratio;
    float mouse_click_strength;
};
