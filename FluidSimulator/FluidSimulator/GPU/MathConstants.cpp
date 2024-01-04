#include "MathConstants.h"
#include <math.h>

void MathConstants::Set(float radius)
{
    poly6_scaling_factor = 4 / (PI * pow(radius, 8));
    spiky_pow3_scaling_factor = 10 / (PI * pow(radius, 5));
    spiky_pow2_scaling_factor = 6 / (PI * pow(radius, 4));
    spiky_pow3_derivative_scaling_factor = 30 / (pow(radius, 5) * PI);
    spiky_pow2_derivative_scaling_factor = 12 / (pow(radius, 4) * PI);
}
