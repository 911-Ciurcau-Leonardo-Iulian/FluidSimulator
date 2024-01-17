#pragma once

#include <mpi.h>
#include <iostream>
#include <thread>
#include <vector>
#include "Vec2.h"

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
};
