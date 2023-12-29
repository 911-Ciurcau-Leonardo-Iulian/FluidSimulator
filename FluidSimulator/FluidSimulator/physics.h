#include "Vec2.h"
#include <imgui.h>

struct SpatialEntry {
    unsigned int index;
    unsigned int hash;
    unsigned int key;
};

class Physics
{
public:
    Int2 GetCell2D(Float2 position, float radius);

    ImU32 HashCell2D(Int2 cell);

    ImU32 KeyFromHash(ImU32 hash, ImU32 tableSize);

    float SmoothingKernelPoly6(float dst, float radius);

    float SpikyKernelPow3(float dst, float radius);

    float SpikyKernelPow2(float dst, float radius);

    float DerivativeSpikyPow3(float dst, float radius);

    float DerivativeSpikyPow2(float dst, float radius);

    float DensityKernel(float dst, float radius);

    float NearDensityKernel(float dst, float radius);

    float DensityDerivative(float dst, float radius);

    float NearDensityDerivative(float dst, float radius);

    float ViscosityKernel(float dst, float radius);

    Float2 CalculateDensity(Float2 pos);

    float PressureFromDensity(float density);

    float NearPressureFromDensity(float nearDensity);

    Float2 ExternalForces(Float2 pos, Float2 velocity);

    void HandleCollisions(ImU32 particleIndex);

    void ExternalForces(SpatialEntry id);

    void UpdateSpatialHash(SpatialEntry id);

    void CalculateDensities(SpatialEntry id);

    void CalculatePressureForce(SpatialEntry id);

    void CalculateViscosity(SpatialEntry id);

    void UpdatePositions(SpatialEntry id);
};
