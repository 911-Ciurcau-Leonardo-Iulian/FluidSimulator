#pragma once

#include "Vec2.h"
#include <imgui.h>
#include <vector>
#include <cmath>

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
    //unsigned int numEntries;
    unsigned int groupWidth;
    unsigned int groupHeight;
    unsigned int stepIndex;
    static const Int2 offsets2D[9];

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
    float currentInteractionInputStrength;
    float interactionInputRadius;
    float interactionInputStrength;

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

    static Int2 GetCell2D(Float2 position, float radius);

    static ImU32 HashCell2D(Int2 cell);

    static ImU32 KeyFromHash(ImU32 hash, ImU32 tableSize);

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

    Float2 CalculateDensityForPos(Float2 pos);

    float PressureFromDensity(float density);

    float NearPressureFromDensity(float nearDensity);

    Float2 ExternalForces(Float2 pos, Float2 velocity);

    void HandleCollisions(ImU32 particleIndex);

    void ExternalForces(int id);

    void UpdateSpatialHash(int id);

    void CalculateDensity(int id);

    void CalculatePressureForce(int id);

    void CalculateViscosity(int id);

    void UpdatePositions(int id);


    int nextPowerOfTwo(unsigned int n)
    {
        int power = 1;
        while (power < n)
            power <<= 1; 
        
        return power;
    }

    // Sorts given buffer of integer values using bitonic merge sort
    // Note: buffer size is not restricted to powers of 2 in this implementation
    void GpuSort()
    {
        // Launch each step of the sorting algorithm (once the previous step is complete)
        // Number of steps = [log2(n) * (log2(n) + 1)] / 2
        // where n = nearest power of 2 that is greater or equal to the number of inputs
        int numStages = (int)std::log2(nextPowerOfTwo(numParticles));

        //launch tasks here VVVVVV

        for (int stageIndex = 0; stageIndex < numStages; stageIndex++)
        {
           for (stepIndex = 0; stepIndex < stageIndex + 1; stepIndex++)
           {
               // Calculate some pattern stuff
               groupWidth = 1 << (stageIndex - stepIndex);
               groupHeight = 2 * groupWidth - 1;
               // Run the sorting step on the GPU
               int bound = nextPowerOfTwo(numParticles) / 2;
               for (int i = 0; i < bound; i++) {
                    Sort(i);
               }
           }
        }
    }


    void GpuSortAndCalculateOffsets()
    {
        GpuSort();
        for (int i = 0; i < numParticles; i++)
        {
            CalculateOffsets(i);
        }
    }
};
