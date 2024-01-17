#pragma once
#include "../Vec2.h"

struct GeneralSettings {
    unsigned int num_particles;
    float gravity;
    float delta_time;
    float collision_damping;
    float smoothing_radius;
    float target_density;
    float pressure_multiplier;
    float near_pressure_multiplier;
    float viscosity_strength;
    float poly6_scaling_factor;
    float spiky_pow3_scaling_factor;
    float spiky_pow2_scaling_factor;
    float spiky_pow3_derivative_scaling_factor;
    float spiky_pow2_derivative_scaling_factor;
    // We don't need padding here
    Float2 interaction_input_point;
    float interaction_input_strength;
    float interaction_input_radius;
    //Float2 obstacleSize;
    //Float2 obstacleCentre;
};
