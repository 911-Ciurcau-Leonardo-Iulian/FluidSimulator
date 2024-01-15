#pragma once
#include "Vec2.h"
#include <vector>

struct ParticleSpawnData
{
    std::vector<Float2> positions;
    std::vector<Float2> velocities;

    ParticleSpawnData(int num) : positions((num)), velocities((num))
    {
    }
};

class ParticleSpawner
{
public:
    int particleCount;

    Float2 initialVelocity;
    Float2 spawnCentre;
    Float2 spawnSize;
    bool showSpawnBoundsGizmos;

    ParticleSpawnData GetSpawnData();

    ParticleSpawner() 
    {
        particleCount = 20;
        initialVelocity = 0;
        spawnCentre = Float2(600, 100);
        spawnSize = 100;
        showSpawnBoundsGizmos = false;
    }

    void OnDrawGizmos()
    {
        // if (showSpawnBoundsGizmos && !Application.isPlaying)
        // {
        //     Gizmos.color = new Color(1, 1, 0, 0.5f);
        //     Gizmos.DrawWireCube(spawnCentre, Vector2.one * spawnSize);
        // }
    }
};

