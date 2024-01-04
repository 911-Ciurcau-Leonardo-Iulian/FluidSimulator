#pragma once
#include "../Vec2.h"

struct GeneralSettings {
    unsigned int numParticles;
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
};
