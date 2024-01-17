#include "physics.h"
#include <vector>
#include <math.h>

static const int NumThreads = 64;

// Sort the given entries by their keys (smallest to largest)
// This is done using bitonic merge sort, and takes multiple iterations
void Physics::Sort(unsigned int id)
{
    unsigned int i = id;

    unsigned int hIndex = i & (groupWidth - 1);
    unsigned int indexLeft = hIndex + (groupHeight + 1) * (i / groupWidth);
    unsigned int rightStepSize = stepIndex == 0 ? groupHeight - 2 * hIndex : (groupHeight + 1) / 2;
    unsigned int indexRight = indexLeft + rightStepSize;

    // Exit if out of bounds (for non-power of 2 input sizes)
    if (indexRight >= numParticles) return;

    unsigned int valueLeft = SpatialIndices[indexLeft].key;
    unsigned int valueRight = SpatialIndices[indexRight].key;

    // Swap entries if value is descending
    if (valueLeft > valueRight)
    {
        SpatialEntry temp = SpatialIndices[indexLeft];
        SpatialIndices[indexLeft] = SpatialIndices[indexRight];
        SpatialIndices[indexRight] = temp;
    }
}


void Physics::ResizeBuffers() {
    Positions.resize(numParticles);
    PredictedPositions.resize(numParticles);
    Velocities.resize(numParticles);
    Densities.resize(numParticles);
    SpatialIndices.resize(numParticles);
    SpatialOffsets.resize(numParticles);
}

// Calculate offsets into the sorted Entries buffer (used for spatial hashing).
// For example, given an Entries buffer sorted by key like so: {2, 2, 2, 3, 6, 6, 9, 9, 9, 9}
// The resulting Offsets calculated here should be:            {-, -, 0, 3, -, -, 4, -, -, 6}
// (where '-' represents elements that won't be read/written)
// 
// Usage example:
// Say we have a particular particle P, and we want to know which particles are in the same grid cell as it.
// First we would calculate the Key of P based on its position. Let's say in this example that Key = 9.
// Next we can look up Offsets[Key] to get: Offsets[9] = 6
// This tells us that SortedEntries[6] is the first particle that's in the same cell as P.
// We can then loop until we reach a particle with a different cell key in order to iterate over all the particles in the cell.
// 
// NOTE: offsets buffer must filled with values equal to (or greater than) its length to ensure that this works correctly

void Physics::CalculateOffsets(unsigned int id)
{
    if (id >= numParticles) { return; }

    unsigned int i = id;
    unsigned int null = numParticles;

    unsigned int key = SpatialIndices[i].key;
    unsigned int keyPrev = i == 0 ? null : SpatialIndices[i - 1].key;

    if (key != keyPrev)
    {
        SpatialOffsets[key] = i;
    }
}

const Int2 Physics::offsets2D[9] =
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
Int2 Physics::GetCell2D(Float2 position, float radius)
{
    return Floor(position / radius);
}

// Hash cell coordinate to a single unsigned integer
ImU32 Physics::HashCell2D(Int2 cell)
{
    ImU32 a = (ImU32)cell.x * hashK1;
    ImU32 b = (ImU32)cell.y * hashK2;
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

Float2 Physics::CalculateDensityForPos(Float2 pos)
{
    Int2 originCell = GetCell2D(pos, smoothingRadius);
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
            SpatialEntry indexData = SpatialIndices[currIndex];
            currIndex++;
            // Exit if no longer looking at correct bin
            if (indexData.key != key) break;
            // Skip if hash does not match
            if (indexData.hash != hash) continue;

            ImU32 neighbourIndex = indexData.index;
            Float2 neighbourPos = PredictedPositions[neighbourIndex];
            Float2 offsetToNeighbour = neighbourPos - pos;
            float sqrDstToNeighbour = Dot(offsetToNeighbour, offsetToNeighbour);

            // Skip if not within radius
            if (sqrDstToNeighbour > sqrRadius) continue;

            // Calculate density and near density
            float dst = sqrt(sqrDstToNeighbour);
            density += DensityKernel(dst, smoothingRadius);
            nearDensity += NearDensityKernel(dst, smoothingRadius);
        }
    }

    return Float2(density, nearDensity);
}

float Physics::PressureFromDensity(float density)
{
    return (density - targetDensity) * pressureMultiplier;
}

float Physics::NearPressureFromDensity(float nearDensity)
{
    return nearPressureMultiplier * nearDensity;
}

Float2 Physics::ExternalForces(Float2 pos, Float2 velocity)
{
    // Gravity
    Float2 gravityAccel = Float2(0, gravity);

    // Input interactions modify gravity
    if (currentInteractionInputStrength != 0) {
        Float2 inputPointOffset = interactionInputPoint - pos;
        float sqrDst = Dot(inputPointOffset, inputPointOffset);
        if (sqrDst < interactionInputRadius * interactionInputRadius)
        {
            float dst = sqrt(sqrDst);
            float edgeT = (dst / interactionInputRadius);
            float centreT = 1 - edgeT;
            Float2 dirToCentre = inputPointOffset / dst;

            float gravityWeight = 1 - (centreT * saturate(currentInteractionInputStrength / 10));
            Float2 accel = gravityAccel * gravityWeight + dirToCentre * centreT * currentInteractionInputStrength;
            accel -= velocity * centreT;
            return accel;
        }
    }

    return gravityAccel;
}


void Physics::HandleCollisions(ImU32 particleIndex)
{
    Float2 pos = Positions[particleIndex];
    Float2 vel = Velocities[particleIndex];

    // Keep particle inside bounds
    const Float2 halfSize = boundsSize;
    Float2 edgeDst = halfSize - Abs(pos);

    const float SPRITE_SIZE = 15.f;

    if (pos.x < SPRITE_SIZE || pos.x > boundsSize.x - SPRITE_SIZE) {
        if (pos.x < SPRITE_SIZE) {
            pos.x = SPRITE_SIZE;
        }
        else {
            pos.x = boundsSize.x - SPRITE_SIZE;
        }
        vel.x *= -1 * collisionDamping;
    }
    if (pos.y < SPRITE_SIZE || pos.y > boundsSize.y - SPRITE_SIZE) {
        if (pos.y < SPRITE_SIZE) {
            pos.y = SPRITE_SIZE;
        }
        else {
            pos.y = boundsSize.y - SPRITE_SIZE;
        }
        vel.y *= -1 * collisionDamping;
    }

    //if (edgeDst.x <= 0)
    //{
    //    pos.x = halfSize.x * sign(pos.x);
    //    vel.x *= -1 * collisionDamping;
    //}
    //if (edgeDst.y <= 0)
    //{
    //    pos.y = halfSize.y * sign(pos.y);
    //    vel.y *= -1 * collisionDamping;
    //}

    // Collide particle against the test obstacle
    //const Float2 obstacleHalfSize = obstacleSize * 0.5;
    //Float2 obstacleEdgeDst = obstacleHalfSize - Abs(pos - obstacleCentre);

    //if (obstacleEdgeDst.x >= 0 && obstacleEdgeDst.y >= 0)
    //{
    //    if (obstacleEdgeDst.x < obstacleEdgeDst.y) {
    //        pos.x = obstacleHalfSize.x * sign(pos.x - obstacleCentre.x) + obstacleCentre.x;
    //        vel.x *= -1 * collisionDamping;
    //    }
    //    else {
    //        pos.y = obstacleHalfSize.y * sign(pos.y - obstacleCentre.y) + obstacleCentre.y;
    //        vel.y *= -1 * collisionDamping;
    //    }
    //}

    // Update position and velocity
    Positions[particleIndex] = pos;
    Velocities[particleIndex] = vel;
}

void Physics::ExternalForces(int id)
{
    if (id >= numParticles) return;

    // External forces Physics::(gravity and input interaction)
    Velocities[id] += ExternalForces(Positions[id], Velocities[id]) * deltaTime;

    // Predict
    const float predictionFactor = 1 / 120.0;
    PredictedPositions[id] = Positions[id] + Velocities[id] * predictionFactor;
}


void Physics::UpdateSpatialHash(int id)
{
	if (id >= numParticles) return;

	// Reset offsets
	SpatialOffsets[id] = numParticles;
	// Update index buffer
	ImU32 index = id;
	Int2 cell = GetCell2D(PredictedPositions[index], smoothingRadius);
	ImU32 hash = HashCell2D(cell);
	ImU32 key = KeyFromHash(hash, numParticles);
    SpatialIndices[id] = { index, hash, key };
}


void Physics::CalculateDensity(int id)
{
    if (id >= numParticles) return;

    Float2 pos = PredictedPositions[id];
    Densities[id] = CalculateDensityForPos(pos);
}


void Physics::CalculatePressureForce(int id)
{
    if (id >= numParticles) return;

    float density = Densities[id][0];
    float densityNear = Densities[id][1];
    float pressure = PressureFromDensity(density);
    float nearPressure = NearPressureFromDensity(densityNear);
    Float2 pressureForce = 0;

    Float2 pos = PredictedPositions[id];
    Int2 originCell = GetCell2D(pos, smoothingRadius);
    float sqrRadius = smoothingRadius * smoothingRadius;

    // Neighbour search
    for (int i = 0; i < 9; i++)
    {
        ImU32 hash = HashCell2D(originCell + offsets2D[i]);
        ImU32 key = KeyFromHash(hash, numParticles);
        ImU32 currIndex = SpatialOffsets[key];

        while (currIndex < numParticles)
        {
            SpatialEntry indexData = SpatialIndices[currIndex];
            currIndex++;
            // Exit if no longer looking at correct bin
            if (indexData.key != key) break;
            // Skip if hash does not match
            if (indexData.hash != hash) continue;

            ImU32 neighbourIndex = indexData.index;
            // Skip if looking at self
            if (neighbourIndex == id) continue;

            Float2 neighbourPos = PredictedPositions[neighbourIndex];
            Float2 offsetToNeighbour = neighbourPos - pos;
            float sqrDstToNeighbour = Dot(offsetToNeighbour, offsetToNeighbour);

            // Skip if not within radius
            if (sqrDstToNeighbour > sqrRadius) continue;

            // Calculate pressure force
            float dst = sqrt(sqrDstToNeighbour);
            Float2 dirToNeighbour = dst > 0 ? offsetToNeighbour / dst : Float2(0, 1);

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

    Float2 acceleration = pressureForce / density;
    Velocities[id] -= acceleration * deltaTime;
}

void Physics::CalculateViscosity(int id)
{
    if (id >= numParticles) return;


    Float2 pos = PredictedPositions[id];
    Int2 originCell = GetCell2D(pos, smoothingRadius);
    float sqrRadius = smoothingRadius * smoothingRadius;

    Float2 viscosityForce = 0;
    Float2 velocity = Velocities[id];

    for (int i = 0; i < 9; i++)
    {
        ImU32 hash = HashCell2D(originCell + offsets2D[i]);
        ImU32 key = KeyFromHash(hash, numParticles);
        ImU32 currIndex = SpatialOffsets[key];

        while (currIndex < numParticles)
        {
            SpatialEntry indexData = SpatialIndices[currIndex];
            currIndex++;
            // Exit if no longer looking at correct bin
            if (indexData.key != key) break;
            // Skip if hash does not match
            if (indexData.hash != hash) continue;

            ImU32 neighbourIndex = indexData.index;
            // Skip if looking at self
            if (neighbourIndex == id) continue;

            Float2 neighbourPos = PredictedPositions[neighbourIndex];
            Float2 offsetToNeighbour = neighbourPos - pos;
            float sqrDstToNeighbour = Dot(offsetToNeighbour, offsetToNeighbour);

            // Skip if not within radius
            if (sqrDstToNeighbour > sqrRadius) continue;

            float dst = sqrt(sqrDstToNeighbour);
            Float2 neighbourVelocity = Velocities[neighbourIndex];
            viscosityForce += (neighbourVelocity - velocity) * ViscosityKernel(dst, smoothingRadius);
        }

    }
    Velocities[id] -= viscosityForce * viscosityStrength * deltaTime;
}


void Physics::UpdatePositions(int id)
{
    if (id >= numParticles) return;

    Positions[id] += Velocities[id] * deltaTime;
    HandleCollisions(id);
}
