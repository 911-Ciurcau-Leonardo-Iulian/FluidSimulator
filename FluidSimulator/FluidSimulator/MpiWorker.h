#pragma once

#include <mpi.h>
#include <iostream>
#include <thread>
#include <vector>
#include "Vec2.h"
#include "physics.h"

class MpiWorker {
public:
    int me;

    MpiWorker(int me) : me(me) {}
    void run();

    void ExternalForces(
        std::vector<Float2>& Velocities,
        std::vector<Float2>& Positions,
        std::vector<Float2>& PredictedPositions,
        float deltaTime,
        int id, float gravity, float currentInteractionInputStrength,
        Float2& interactionInputPoint, float interactionInputRadius);

    Float2 ExternalForces(Float2& pos, Float2& velocity, float gravity, float currentInteractionInputStrength,
        Float2& interactionInputPoint, float interactionInputRadius);

    void CalculateDensity(int id,
        std::vector<Float2>& PredictedPositions,
        std::vector<Float2>& Densities,
        float smoothingRadius,
        std::vector<ImU32>& SpatialOffsets,
        ImU32 numParticles,
        std::vector<SpatialEntry>& SpatialIndices,
        float SpikyPow2ScalingFactor,
        float SpikyPow3ScalingFactor
    );

    Float2 CalculateDensityForPos(Float2 pos,
        float smoothingRadius,
        std::vector<ImU32>& SpatialOffsets,
        ImU32 numParticles,
        std::vector<SpatialEntry>& SpatialIndices,
        std::vector<Float2>& PredictedPositions,
        float SpikyPow2ScalingFactor,
        float SpikyPow3ScalingFactor
    );

    float DensityKernel(float dst, float radius, float SpikyPow2ScalingFactor);
    float NearDensityKernel(float dst, float radius, float SpikyPow3ScalingFactor);
};
