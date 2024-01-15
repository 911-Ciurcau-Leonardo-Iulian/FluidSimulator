#include "ParticleSpawner.h"
#include "time.h"
#include <cmath>
#include <iostream>

ParticleSpawnData ParticleSpawner::GetSpawnData() 
{
    ParticleSpawnData data(particleCount);
    srand(time(NULL));

    Float2 s = spawnSize;
    int numX = std::ceil(std::sqrt(s.x / s.y * particleCount + (s.x - s.y) * (s.x - s.y) / (4 * s.y * s.y)) - (s.x - s.y) / (2 * s.y));
    int numY = std::ceil(particleCount / (float)numX);
    int i = 0;
    std::cout << "numX " << numX << "numY " << numY << "\n\n";

    for (int y = 0; y < numY; y++)
    {
        for (int x = 0; x < numX; x++)
        {
            if (i >= particleCount) break;

            float tx = numX <= 1 ? 0.5f : x / (numX - 1.0f);
            float ty = numY <= 1 ? 0.5f : y / (numY - 1.0f);

            data.positions[i] = Float2((tx - 0.5f) * spawnSize.x, (ty - 0.5f) * spawnSize.y) + spawnCentre;
            data.velocities[i] = initialVelocity;
            i++;
        }
    }

    return data;
}