#include "physics_vector2d.h"
#include "physics_vector3d.h"
#include <imgui.h>

class Physics
{
    PhysicsVector2D<int> GetCell2D(PhysicsVector2D<float> position, float radius);

    ImU32 HashCell2D(PhysicsVector2D<int>  cell);

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

    PhysicsVector2D<float> CalculateDensity(PhysicsVector2D<float> pos);

    float PressureFromDensity(float density);

    float NearPressureFromDensity(float nearDensity);

    PhysicsVector2D<float> ExternalForces(PhysicsVector2D<float> pos, PhysicsVector2D<float> velocity);

    void HandleCollisions(ImU32 particleIndex);

    void ExternalForces(PhysicsVector3D<ImU32> id);

    void UpdateSpatialHash(PhysicsVector3D<ImU32> id);

    void CalculateDensities(PhysicsVector3D<ImU32> id);

    void CalculatePressureForce(PhysicsVector3D<ImU32> id);

    void CalculateViscosity(PhysicsVector3D<ImU32> id);

    void UpdatePositions(PhysicsVector3D<ImU32> id);
};
