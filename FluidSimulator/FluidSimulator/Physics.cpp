#include "physics.h"
#include <vector>
#include <imgui.h>
#include <math.h>
#include <tuple>

static const int NumThreads = 64;

// Buffers
std::vector<ImVec2> Positions;
std::vector<ImVec2> PredictedPositions;
std::vector<ImVec2> Velocities;
std::vector<ImVec2> Densities; // Density, Near Density
std::vector<std::tuple<ImU32, ImU32, ImU32>> SpatialIndices; // used for spatial hashing
std::vector<ImU32> SpatialOffsets; // used for spatial hashing

// Settings
const ImU32 numParticles;
const float gravity;
const float deltaTime;
const float collisionDamping;
const float smoothingRadius;
const float targetDensity;
const float pressureMultiplier;
const float nearPressureMultiplier;
const float viscosityStrength;
const ImVec2 boundsSize;
const ImVec2 interactionInputPoint;
const float interactionInputStrength;
const float interactionInputRadius;

const ImVec2 obstacleSize;
const ImVec2 obstacleCentre;

const float Poly6ScalingFactor;
const float SpikyPow3ScalingFactor;
const float SpikyPow2ScalingFactor;
const float SpikyPow3DerivativeScalingFactor;
const float SpikyPow2DerivativeScalingFactor;


static const std::tuple<int, int>  offsets2D[9] =
{
    {-1, 1},
    {0, 1},
    {1, 1},
    {-1, 0},
    {0, 0},
    {1, 0},
    {-1, -1},
    {0, -1},
    {1, -1},
};

// Constants used for hashing
static const ImU32 hashK1 = 15823;
static const ImU32 hashK2 = 9737333;

// Convert floating point position into an integer cell coordinate
std::tuple<int, int>  GetCell2D(ImVec2 position, float radius)
{
    return (std::tuple<int, int>) floor(position / radius);
}

// Hash cell coordinate to a single unsigned integer
ImU32 HashCell2D(std::tuple<int, int>  cell)
{
    cell = (std::tuple<int, int>)cell;
    ImU32 a = cell.x * hashK1;
    ImU32 b = cell.y * hashK2;
    return (a + b);
}

ImU32 KeyFromHash(ImU32 hash, ImU32 tableSize)
{
    return hash % tableSize;
}



float SmoothingKernelPoly6(float dst, float radius)
{
    if (dst < radius)
    {
        float v = radius * radius - dst * dst;
        return v * v * v * Poly6ScalingFactor;
    }
    return 0;
}

float SpikyKernelPow3(float dst, float radius)
{
    if (dst < radius)
    {
        float v = radius - dst;
        return v * v * v * SpikyPow3ScalingFactor;
    }
    return 0;
}

float SpikyKernelPow2(float dst, float radius)
{
    if (dst < radius)
    {
        float v = radius - dst;
        return v * v * SpikyPow2ScalingFactor;
    }
    return 0;
}

float DerivativeSpikyPow3(float dst, float radius)
{
    if (dst <= radius)
    {
        float v = radius - dst;
        return -v * v * SpikyPow3DerivativeScalingFactor;
    }
    return 0;
}

float DerivativeSpikyPow2(float dst, float radius)
{
    if (dst <= radius)
    {
        float v = radius - dst;
        return -v * SpikyPow2DerivativeScalingFactor;
    }
    return 0;
}

float DensityKernel(float dst, float radius)
{
    return SpikyKernelPow2(dst, radius);
}

float NearDensityKernel(float dst, float radius)
{
    return SpikyKernelPow3(dst, radius);
}

float DensityDerivative(float dst, float radius)
{
    return DerivativeSpikyPow2(dst, radius);
}

float NearDensityDerivative(float dst, float radius)
{
    return DerivativeSpikyPow3(dst, radius);
}

float ViscosityKernel(float dst, float radius)
{
    return SmoothingKernelPoly6(dst, smoothingRadius);
}

ImVec2 CalculateDensity(ImVec2 pos)
{
    std::tuple<int, int> originCell = GetCell2D(pos, smoothingRadius);
    float sqrRadius = smoothingRadius * smoothingRadius;
    float density = 0;
    float nearDensity = 0;

    // Neighbour search
    for (int i = 0; i < 9; i++)
    {
        ImU32 hash = HashCell2D(originCell + offsets2D[i]);
        ImU32 key = KeyFromHash(hash, numParticles);
        ImU32 currIndex = SpatialOffsets[key];

        while (currIndex < numParticles)
        {
            std::tuple<ImU32, ImU32, ImU32> indexData = SpatialIndices[currIndex];
            currIndex++;
            // Exit if no longer looking at correct bin
            if (indexData[2] != key) break;
            // Skip if hash does not match
            if (indexData[1] != hash) continue;

            ImU32 neighbourIndex = indexData[0];
            ImVec2 neighbourPos = PredictedPositions[neighbourIndex];
            ImVec2 offsetToNeighbour = neighbourPos - pos;
            float sqrDstToNeighbour = dot(offsetToNeighbour, offsetToNeighbour);

            // Skip if not within radius
            if (sqrDstToNeighbour > sqrRadius) continue;

            // Calculate density and near density
            float dst = sqrt(sqrDstToNeighbour);
            density += DensityKernel(dst, smoothingRadius);
            nearDensity += NearDensityKernel(dst, smoothingRadius);
        }
    }

    return ImVec2(density, nearDensity);
}

float PressureFromDensity(float density)
{
    return (density - targetDensity) * pressureMultiplier;
}

float NearPressureFromDensity(float nearDensity)
{
    return nearPressureMultiplier * nearDensity;
}

ImVec2 ExternalForces(ImVec2 pos, ImVec2 velocity)
{
    // Gravity
    ImVec2 gravityAccel = ImVec2(0, gravity);

    // Input interactions modify gravity
    if (interactionInputStrength != 0) {
        ImVec2 inputPointOffset = interactionInputPoint - pos;
        float sqrDst = dot(inputPointOffset, inputPointOffset);
        if (sqrDst < interactionInputRadius * interactionInputRadius)
        {
            float dst = sqrt(sqrDst);
            float edgeT = (dst / interactionInputRadius);
            float centreT = 1 - edgeT;
            ImVec2 dirToCentre = inputPointOffset / dst;

            float gravityWeight = 1 - (centreT * saturate(interactionInputStrength / 10));
            ImVec2 accel = gravityAccel * gravityWeight + dirToCentre * centreT * interactionInputStrength;
            accel -= velocity * centreT;
            return accel;
        }
    }

    return gravityAccel;
}


void HandleCollisions(ImU32 particleIndex)
{
    ImVec2 pos = Positions[particleIndex];
    ImVec2 vel = Velocities[particleIndex];

    // Keep particle inside bounds
    const ImVec2 halfSize = boundsSize * 0.5;
    ImVec2 edgeDst = halfSize - abs(pos);

    if (edgeDst.x <= 0)
    {
        pos.x = halfSize.x * sign(pos.x);
        vel.x *= -1 * collisionDamping;
    }
    if (edgeDst.y <= 0)
    {
        pos.y = halfSize.y * sign(pos.y);
        vel.y *= -1 * collisionDamping;
    }

    // Collide particle against the test obstacle
    const ImVec2 obstacleHalfSize = obstacleSize * 0.5;
    ImVec2 obstacleEdgeDst = obstacleHalfSize - abs(pos - obstacleCentre);

    if (obstacleEdgeDst.x >= 0 && obstacleEdgeDst.y >= 0)
    {
        if (obstacleEdgeDst.x < obstacleEdgeDst.y) {
            pos.x = obstacleHalfSize.x * sign(pos.x - obstacleCentre.x) + obstacleCentre.x;
            vel.x *= -1 * collisionDamping;
        }
        else {
            pos.y = obstacleHalfSize.y * sign(pos.y - obstacleCentre.y) + obstacleCentre.y;
            vel.y *= -1 * collisionDamping;
        }
    }

    // Update position and velocity
    Positions[particleIndex] = pos;
    Velocities[particleIndex] = vel;
}

[numthreads(NumThreads, 1, 1)]
void ExternalForces(std::tuple<ImU32, ImU32, ImU32> id : SV_DispatchThreadID)
{
    if (id.x >= numParticles) return;

    // External forces (gravity and input interaction)
    Velocities[id.x] += ExternalForces(Positions[id.x], Velocities[id.x]) * deltaTime;

    // Predict
    const float predictionFactor = 1 / 120.0;
    PredictedPositions[id.x] = Positions[id.x] + Velocities[id.x] * predictionFactor;
}

[numthreads(NumThreads, 1, 1)]
void UpdateSpatialHash(std::tuple<ImU32, ImU32, ImU32> id : SV_DispatchThreadID)
{
    if (id.x >= numParticles) return;

    // Reset offsets
    SpatialOffsets[id.x] = numParticles;
    // Update index buffer
    ImU32 index = id.x;
    std::tuple<int, int> cell = GetCell2D(PredictedPositions[index], smoothingRadius);
    ImU32 hash = HashCell2D(cell);
    ImU32 key = KeyFromHash(hash, numParticles);
    SpatialIndices[id.x] = uint3(index, hash, key);
}

[numthreads(NumThreads, 1, 1)]
void CalculateDensities(std::tuple<ImU32, ImU32, ImU32> id : SV_DispatchThreadID)
{
    if (id.x >= numParticles) return;

    ImVec2 pos = PredictedPositions[id.x];
    Densities[id.x] = CalculateDensity(pos);
}

[numthreads(NumThreads, 1, 1)]
void CalculatePressureForce(std::tuple<ImU32, ImU32, ImU32> id : SV_DispatchThreadID)
{
    if (id.x >= numParticles) return;

    float density = Densities[id.x][0];
    float densityNear = Densities[id.x][1];
    float pressure = PressureFromDensity(density);
    float nearPressure = NearPressureFromDensity(densityNear);
    ImVec2 pressureForce = 0;

    ImVec2 pos = PredictedPositions[id.x];
    std::tuple<int, int>originCell = GetCell2D(pos, smoothingRadius);
    float sqrRadius = smoothingRadius * smoothingRadius;

    // Neighbour search
    for (int i = 0; i < 9; i++)
    {
        ImU32 hash = HashCell2D(originCell + offsets2D[i]);
        ImU32 key = KeyFromHash(hash, numParticles);
        ImU32 currIndex = SpatialOffsets[key];

        while (currIndex < numParticles)
        {
            std::tuple<ImU32, ImU32, ImU32> indexData = SpatialIndices[currIndex];
            currIndex++;
            // Exit if no longer looking at correct bin
            if (indexData[2] != key) break;
            // Skip if hash does not match
            if (indexData[1] != hash) continue;

            ImU32 neighbourIndex = indexData[0];
            // Skip if looking at self
            if (neighbourIndex == id.x) continue;

            ImVec2 neighbourPos = PredictedPositions[neighbourIndex];
            ImVec2 offsetToNeighbour = neighbourPos - pos;
            float sqrDstToNeighbour = dot(offsetToNeighbour, offsetToNeighbour);

            // Skip if not within radius
            if (sqrDstToNeighbour > sqrRadius) continue;

            // Calculate pressure force
            float dst = sqrt(sqrDstToNeighbour);
            ImVec2 dirToNeighbour = dst > 0 ? offsetToNeighbour / dst : ImVec2(0, 1);

            float neighbourDensity = Densities[neighbourIndex][0];
            float neighbourNearDensity = Densities[neighbourIndex][1];
            float neighbourPressure = PressureFromDensity(neighbourDensity);
            float neighbourNearPressure = NearPressureFromDensity(neighbourNearDensity);

            float sharedPressure = (pressure + neighbourPressure) * 0.5;
            float sharedNearPressure = (nearPressure + neighbourNearPressure) * 0.5;

            pressureForce += dirToNeighbour * DensityDerivative(dst, smoothingRadius) * sharedPressure / neighbourDensity;
            pressureForce += dirToNeighbour * NearDensityDerivative(dst, smoothingRadius) * sharedNearPressure / neighbourNearDensity;
        }
    }

    ImVec2 acceleration = pressureForce / density;
    Velocities[id.x] += acceleration * deltaTime;//
}



[numthreads(NumThreads, 1, 1)]
void CalculateViscosity(std::tuple<ImU32, ImU32, ImU32> id : SV_DispatchThreadID)
{
    if (id.x >= numParticles) return;


    ImVec2 pos = PredictedPositions[id.x];
    std::tuple<int, int>originCell = GetCell2D(pos, smoothingRadius);
    float sqrRadius = smoothingRadius * smoothingRadius;

    ImVec2 viscosityForce = 0;
    ImVec2 velocity = Velocities[id.x];

    for (int i = 0; i < 9; i++)
    {
        ImU32 hash = HashCell2D(originCell + offsets2D[i]);
        ImU32 key = KeyFromHash(hash, numParticles);
        ImU32 currIndex = SpatialOffsets[key];

        while (currIndex < numParticles)
        {
            std::tuple<ImU32, ImU32, ImU32> indexData = SpatialIndices[currIndex];
            currIndex++;
            // Exit if no longer looking at correct bin
            if (indexData[2] != key) break;
            // Skip if hash does not match
            if (indexData[1] != hash) continue;

            ImU32 neighbourIndex = indexData[0];
            // Skip if looking at self
            if (neighbourIndex == id.x) continue;

            ImVec2 neighbourPos = PredictedPositions[neighbourIndex];
            ImVec2 offsetToNeighbour = neighbourPos - pos;
            float sqrDstToNeighbour = dot(offsetToNeighbour, offsetToNeighbour);

            // Skip if not within radius
            if (sqrDstToNeighbour > sqrRadius) continue;

            float dst = sqrt(sqrDstToNeighbour);
            ImVec2 neighbourVelocity = Velocities[neighbourIndex];
            viscosityForce += (neighbourVelocity - velocity) * ViscosityKernel(dst, smoothingRadius);
        }

    }
    Velocities[id.x] += viscosityForce * viscosityStrength * deltaTime;
}

[numthreads(NumThreads, 1, 1)]
void UpdatePositions(std::tuple<ImU32, ImU32, ImU32> id : SV_DispatchThreadID)
{
    if (id.x >= numParticles) return;

    Positions[id.x] += Velocities[id.x] * deltaTime;
    HandleCollisions(id.x);
}
