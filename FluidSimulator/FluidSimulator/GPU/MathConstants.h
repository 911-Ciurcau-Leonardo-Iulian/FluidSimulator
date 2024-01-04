#pragma once

#define PI (3.14159265358979323846f)

struct MathConstants {
    void Set(float radius);

    float poly6_scaling_factor;
    float spiky_pow3_scaling_factor;
    float spiky_pow2_scaling_factor;
    float spiky_pow3_derivative_scaling_factor;
    float spiky_pow2_derivative_scaling_factor;
};
