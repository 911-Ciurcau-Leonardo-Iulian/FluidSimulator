#include "Vec2.h"
#include <imgui.h>
#include <vector>

struct SpatialEntry {
    unsigned int index;
    unsigned int hash;
    unsigned int key;
};

struct Entry
{
    unsigned int originalIndex;
    unsigned int hash;
    unsigned int key;
};

struct Physics
{
    //GPUSort
    std::vector<Entry> Entries;
    std::vector<unsigned int> Offsets;
    unsigned int numEntries;
    unsigned int groupWidth;
    unsigned int groupHeight;
    unsigned int stepIndex;

    //Simulation params
    ImU32 numParticles;
    float gravity;
    float deltaTime;
    float collisionDamping;
    float smoothingRadius;
    float targetDensity;
    float pressureMultiplier;
    float nearPressureMultiplier;
    float viscosityStrength;
    Float2 boundsSize;
    Float2 interactionInputPoint;
    float interactionInputStrength;
    float interactionInputRadius;

    Float2 obstacleSize;
    Float2 obstacleCentre;

    float Poly6ScalingFactor;
    float SpikyPow3ScalingFactor;
    float SpikyPow2ScalingFactor;
    float SpikyPow3DerivativeScalingFactor;
    float SpikyPow2DerivativeScalingFactor;

    std::vector<Float2> Positions;
    std::vector<Float2> PredictedPositions;
    std::vector<Float2> Velocities;
    std::vector<Float2> Densities; // Density, Near Density
    std::vector<SpatialEntry> SpatialIndices; // used for spatial hashing
    std::vector<ImU32> SpatialOffsets; // used for spatial hashing

    void CalculateOffsets(unsigned int id);

    void ResizeBuffers();

    void Sort(unsigned int id);

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
