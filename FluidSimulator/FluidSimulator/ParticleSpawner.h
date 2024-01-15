#pragma once
#include "Vec2.h"
#include <vector>

struct ParticleSpawnData
{
    std::vector<Float2> positions;
    std::vector<Float2> velocities;

    ParticleSpawnData(int num)
    {
        positions = new Float2[num];
        velocities = new Float2[num];
    }
};

class ParticleSpawner
{
public:
    int particleCount;

    Vector2 initialVelocity;
    Vector2 spawnCentre;
    Vector2 spawnSize;
    float jitterStr;
    bool showSpawnBoundsGizmos;

    ParticleSpawnData GetSpawnData();

    void OnDrawGizmos()
    {
        if (showSpawnBoundsGizmos && !Application.isPlaying)
        {
            Gizmos.color = new Color(1, 1, 0, 0.5f);
            Gizmos.DrawWireCube(spawnCentre, Vector2.one * spawnSize);
        }
    }
};

