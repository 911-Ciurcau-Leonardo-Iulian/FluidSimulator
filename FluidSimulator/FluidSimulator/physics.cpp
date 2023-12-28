#include "physics.h"
#include <vector>
#include <math.h>

static const int NumThreads = 64;

// Buffers
std::vector<PhysicsVector2D<float>> Positions;
std::vector<PhysicsVector2D<float>> PredictedPositions;
std::vector<PhysicsVector2D<float>> Velocities;
std::vector<PhysicsVector2D<float>> Densities; // Density, Near Density
std::vector<PhysicsVector3D<ImU32>> SpatialIndices; // used for spatial hashing
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
const PhysicsVector2D<float> boundsSize;
const PhysicsVector2D<float> interactionInputPoint;
const float interactionInputStrength;
const float interactionInputRadius;

const PhysicsVector2D<float> obstacleSize;
const PhysicsVector2D<float> obstacleCentre;

const float Poly6ScalingFactor;
const float SpikyPow3ScalingFactor;
const float SpikyPow2ScalingFactor;
const float SpikyPow3DerivativeScalingFactor;
const float SpikyPow2DerivativeScalingFactor;


static const PhysicsVector2D<int>  offsets2D[9] =
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
PhysicsVector2D<int> Physics::GetCell2D(PhysicsVector2D<float> position, float radius)
{
    return (PhysicsVector2D<int>) PhysicsVector2D<float>::floor(position / radius);
}

// Hash cell coordinate to a single unsigned integer
ImU32 Physics::HashCell2D(PhysicsVector2D<int>  cell)
{
    cell = (PhysicsVector2D<int>)cell;
    ImU32 a = cell.x * hashK1;
    ImU32 b = cell.y * hashK2;
    return (a + b);
}

ImU32 Physics::KeyFromHash(ImU32 hash, ImU32 tableSize)
{
    return hash % tableSize;
}



float Physics::SmoothingKernelPoly6(float dst, float radius)
{
    if (dst < radius)
    {
        float v = radius * radius - dst * dst;
        return v * v * v * Poly6ScalingFactor;
    }
    return 0;
}

float Physics::SpikyKernelPow3(float dst, float radius)
{
    if (dst < radius)
    {
        float v = radius - dst;
        return v * v * v * SpikyPow3ScalingFactor;
    }
    return 0;
}

float Physics::SpikyKernelPow2(float dst, float radius)
{
    if (dst < radius)
    {
        float v = radius - dst;
        return v * v * SpikyPow2ScalingFactor;
    }
    return 0;
}

float Physics::DerivativeSpikyPow3(float dst, float radius)
{
    if (dst <= radius)
    {
        float v = radius - dst;
        return -v * v * SpikyPow3DerivativeScalingFactor;
    }
    return 0;
}

float Physics::DerivativeSpikyPow2(float dst, float radius)
{
    if (dst <= radius)
    {
        float v = radius - dst;
        return -v * SpikyPow2DerivativeScalingFactor;
    }
    return 0;
}

float Physics::DensityKernel(float dst, float radius)
{
    return SpikyKernelPow2(dst, radius);
}

float Physics::NearDensityKernel(float dst, float radius)
{
    return SpikyKernelPow3(dst, radius);
}

float Physics::DensityDerivative(float dst, float radius)
{
    return DerivativeSpikyPow2(dst, radius);
}

float Physics::NearDensityDerivative(float dst, float radius)
{
    return DerivativeSpikyPow3(dst, radius);
}

float Physics::ViscosityKernel(float dst, float radius)
{
    return SmoothingKernelPoly6(dst, smoothingRadius);
}

PhysicsVector2D<float> Physics::CalculateDensity(PhysicsVector2D<float> pos)
{
    PhysicsVector2D<int> originCell = GetCell2D(pos, smoothingRadius);
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
            PhysicsVector3D<ImU32> indexData = SpatialIndices[currIndex];
            currIndex++;
            // Exit if no longer looking at correct bin
            if (indexData[2] != key) break;
            // Skip if hash does not match
            if (indexData[1] != hash) continue;

            ImU32 neighbourIndex = indexData[0];
            PhysicsVector2D<float> neighbourPos = PredictedPositions[neighbourIndex];
            PhysicsVector2D<float> offsetToNeighbour = neighbourPos - pos;
            float sqrDstToNeighbour = PhysicsVector2D<float>::dot(offsetToNeighbour, offsetToNeighbour);

            // Skip if not within radius
            if (sqrDstToNeighbour > sqrRadius) continue;

            // Calculate density and near density
            float dst = sqrt(sqrDstToNeighbour);
            density += DensityKernel(dst, smoothingRadius);
            nearDensity += NearDensityKernel(dst, smoothingRadius);
        }
    }

    return PhysicsVector2D<float>(density, nearDensity);
}

float Physics::PressureFromDensity(float density)
{
    return (density - targetDensity) * pressureMultiplier;
}

float Physics::NearPressureFromDensity(float nearDensity)
{
    return nearPressureMultiplier * nearDensity;
}

PhysicsVector2D<float> Physics::ExternalForces(PhysicsVector2D<float> pos, PhysicsVector2D<float> velocity)
{
    // Gravity
    PhysicsVector2D<float> gravityAccel = PhysicsVector2D<float>(0, gravity);

    // Input interactions modify gravity
    if (interactionInputStrength != 0) {
        PhysicsVector2D<float> inputPointOffset = interactionInputPoint - pos;
        float sqrDst = PhysicsVector2D<float>::dot(inputPointOffset, inputPointOffset);
        if (sqrDst < interactionInputRadius * interactionInputRadius)
        {
            float dst = sqrt(sqrDst);
            float edgeT = (dst / interactionInputRadius);
            float centreT = 1 - edgeT;
            PhysicsVector2D<float> dirToCentre = inputPointOffset / dst;

            float gravityWeight = 1 - (centreT * saturate(interactionInputStrength / 10));
            PhysicsVector2D<float> accel = gravityAccel * gravityWeight + dirToCentre * centreT * interactionInputStrength;
            accel -= velocity * centreT;
            return accel;
        }
    }

    return gravityAccel;
}


void Physics::HandleCollisions(ImU32 particleIndex)
{
    PhysicsVector2D<float> pos = Positions[particleIndex];
    PhysicsVector2D<float> vel = Velocities[particleIndex];

    // Keep particle inside bounds
    const PhysicsVector2D<float> halfSize = boundsSize * 0.5;
    PhysicsVector2D<float> edgeDst = halfSize - PhysicsVector2D<float>::abs(pos);

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
    const PhysicsVector2D<float> obstacleHalfSize = obstacleSize * 0.5;
    PhysicsVector2D<float> obstacleEdgeDst = obstacleHalfSize - PhysicsVector2D<float>::abs(pos - obstacleCentre);

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

void Physics::ExternalForces(PhysicsVector3D<ImU32> id)
{
    if (id.x >= numParticles) return;

    // External forces Physics::(gravity and input interaction)
    Velocities[id.x] += ExternalForces(Positions[id.x], Velocities[id.x]) * deltaTime;

    // Predict
    const float predictionFactor = 1 / 120.0;
    PredictedPositions[id.x] = Positions[id.x] + Velocities[id.x] * predictionFactor;
}


void Physics::UpdateSpatialHash(PhysicsVector3D<ImU32> id)
{
	if (id.x >= numParticles) return;

	// Reset offsets
	SpatialOffsets[id.x] = numParticles;
	// Update index buffer
	ImU32 index = id.x;
	PhysicsVector2D<int> cell = GetCell2D(PredictedPositions[index], smoothingRadius);
	ImU32 hash = HashCell2D(cell);
	ImU32 key = KeyFromHash(hash, numParticles);
	SpatialIndices[id.x] = PhysicsVector3D<ImU32>(index, hash, key);
}


void Physics::CalculateDensities(PhysicsVector3D<ImU32> id)
{
    if (id.x >= numParticles) return;

    PhysicsVector2D<float> pos = PredictedPositions[id.x];
    Densities[id.x] = CalculateDensity(pos);
}


void Physics::CalculatePressureForce(PhysicsVector3D<ImU32> id)
{
    if (id.x >= numParticles) return;

    float density = Densities[id.x][0];
    float densityNear = Densities[id.x][1];
    float pressure = PressureFromDensity(density);
    float nearPressure = NearPressureFromDensity(densityNear);
    PhysicsVector2D<float> pressureForce = 0;

    PhysicsVector2D<float> pos = PredictedPositions[id.x];
    PhysicsVector2D<int>originCell = GetCell2D(pos, smoothingRadius);
    float sqrRadius = smoothingRadius * smoothingRadius;

    // Neighbour search
    for (int i = 0; i < 9; i++)
    {
        ImU32 hash = HashCell2D(originCell + offsets2D[i]);
        ImU32 key = KeyFromHash(hash, numParticles);
        ImU32 currIndex = SpatialOffsets[key];

        while (currIndex < numParticles)
        {
            PhysicsVector3D<ImU32> indexData = SpatialIndices[currIndex];
            currIndex++;
            // Exit if no longer looking at correct bin
            if (indexData[2] != key) break;
            // Skip if hash does not match
            if (indexData[1] != hash) continue;

            ImU32 neighbourIndex = indexData[0];
            // Skip if looking at self
            if (neighbourIndex == id.x) continue;

            PhysicsVector2D<float> neighbourPos = PredictedPositions[neighbourIndex];
            PhysicsVector2D<float> offsetToNeighbour = neighbourPos - pos;
            float sqrDstToNeighbour = PhysicsVector2D<float>::dot(offsetToNeighbour, offsetToNeighbour);

            // Skip if not within radius
            if (sqrDstToNeighbour > sqrRadius) continue;

            // Calculate pressure force
            float dst = sqrt(sqrDstToNeighbour);
            PhysicsVector2D<float> dirToNeighbour = dst > 0 ? offsetToNeighbour / dst : PhysicsVector2D<float>(0, 1);

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

    PhysicsVector2D<float> acceleration = pressureForce / density;
    Velocities[id.x] += acceleration * deltaTime;//
}




void Physics::CalculateViscosity(PhysicsVector3D<ImU32> id)
{
    if (id.x >= numParticles) return;


    PhysicsVector2D<float> pos = PredictedPositions[id.x];
    PhysicsVector2D<int>originCell = GetCell2D(pos, smoothingRadius);
    float sqrRadius = smoothingRadius * smoothingRadius;

    PhysicsVector2D<float> viscosityForce = 0;
    PhysicsVector2D<float> velocity = Velocities[id.x];

    for (int i = 0; i < 9; i++)
    {
        ImU32 hash = HashCell2D(originCell + offsets2D[i]);
        ImU32 key = KeyFromHash(hash, numParticles);
        ImU32 currIndex = SpatialOffsets[key];

        while (currIndex < numParticles)
        {
            PhysicsVector3D<ImU32> indexData = SpatialIndices[currIndex];
            currIndex++;
            // Exit if no longer looking at correct bin
            if (indexData[2] != key) break;
            // Skip if hash does not match
            if (indexData[1] != hash) continue;

            ImU32 neighbourIndex = indexData[0];
            // Skip if looking at self
            if (neighbourIndex == id.x) continue;

            PhysicsVector2D<float> neighbourPos = PredictedPositions[neighbourIndex];
            PhysicsVector2D<float> offsetToNeighbour = neighbourPos - pos;
            float sqrDstToNeighbour = PhysicsVector2D<float>::dot(offsetToNeighbour, offsetToNeighbour);

            // Skip if not within radius
            if (sqrDstToNeighbour > sqrRadius) continue;

            float dst = sqrt(sqrDstToNeighbour);
            PhysicsVector2D<float> neighbourVelocity = Velocities[neighbourIndex];
            viscosityForce += (neighbourVelocity - velocity) * ViscosityKernel(dst, smoothingRadius);
        }

    }
    Velocities[id.x] += viscosityForce * viscosityStrength * deltaTime;
}


void Physics::UpdatePositions(PhysicsVector3D<ImU32> id)
{
    if (id.x >= numParticles) return;

    Positions[id.x] += Velocities[id.x] * deltaTime;
    HandleCollisions(id.x);
}
