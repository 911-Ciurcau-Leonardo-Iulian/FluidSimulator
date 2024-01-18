#include "MpiWorker.h"

void MpiWorker::run()
{
    MPI_Status status;
    int batchSize;
    std::vector<Float2> Velocities, Positions, PredictedPositions, Densities;
    float deltaTime, gravity, currentInteractionInputStrength, interactionInputRadius,
        smoothingRadius, SpikyPow2ScalingFactor, SpikyPow3ScalingFactor,
        targetDensity, pressureMultiplier, nearPressureMultiplier,
        SpikyPow2DerivativeScalingFactor, spikyPow3DerivativeScalingFactor,
        Poly6ScalingFactor, viscosityStrength, collisionDamping;
    Float2 interactionInputPoint, boundsSize;
    std::vector<ImU32> SpatialOffsets;
    ImU32 numParticles;
    std::vector<SpatialEntry> SpatialIndices;

    while (true)
    {
        // ExternalForces


        MPI_Recv(&batchSize, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        Velocities.resize(numParticles);
        MPI_Recv(Velocities.data(), batchSize * 2, MPI_FLOAT, 0, 2, MPI_COMM_WORLD, &status);
        Positions.resize(numParticles);
        MPI_Recv(Positions.data(), batchSize * 2, MPI_FLOAT, 0, 3, MPI_COMM_WORLD, &status);
        MPI_Recv(&deltaTime , 1, MPI_FLOAT, 0, 4, MPI_COMM_WORLD, &status);
        MPI_Recv(&gravity , 1, MPI_FLOAT, 0, 5, MPI_COMM_WORLD, &status);
        MPI_Recv(&currentInteractionInputStrength, 1, MPI_FLOAT, 0, 6, MPI_COMM_WORLD, &status);
        MPI_Recv(&interactionInputPoint, 2, MPI_FLOAT, 0, 7, MPI_COMM_WORLD, &status);
        MPI_Recv(&interactionInputRadius, 1, MPI_FLOAT, 0, 8, MPI_COMM_WORLD, &status);

        PredictedPositions.resize(numParticles);

        for (int i = 0; i < batchSize; i++)
        {
            ExternalForces(
                Velocities,
                Positions,
                PredictedPositions,
                deltaTime,
                i, gravity, currentInteractionInputStrength,
                interactionInputPoint, interactionInputRadius);
        }

        MPI_Send(Velocities.data(), batchSize * 2, MPI_FLOAT, 0, 9, MPI_COMM_WORLD);
        MPI_Send(PredictedPositions.data(), batchSize * 2, MPI_FLOAT, 0, 10, MPI_COMM_WORLD);


        // CalculateDensity

        MPI_Recv(PredictedPositions.data(), numParticles * 2, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);


        MPI_Recv(&smoothingRadius, 1, MPI_FLOAT, 0, 11, MPI_COMM_WORLD, &status);
        MPI_Recv(&numParticles, 1, MPI_UINT32_T, 0, 12, MPI_COMM_WORLD, &status);
        SpatialOffsets.resize(numParticles);
        MPI_Recv(SpatialOffsets.data(), numParticles, MPI_UINT32_T, 0, 13, MPI_COMM_WORLD, &status);
        SpatialIndices.resize(numParticles);
        MPI_Recv(SpatialIndices.data(), numParticles * 3, MPI_UINT32_T, 0, 14, MPI_COMM_WORLD, &status);
        MPI_Recv(&SpikyPow2ScalingFactor, 1, MPI_FLOAT, 0, 15, MPI_COMM_WORLD, &status);
        MPI_Recv(&SpikyPow3ScalingFactor, 1, MPI_FLOAT, 0, 16, MPI_COMM_WORLD, &status);


        Densities.resize(numParticles);

        for (int i = 0; i < batchSize; i++)
        {
            CalculateDensity(
                i,
                PredictedPositions,
                Densities,
                smoothingRadius,
                SpatialOffsets,
                numParticles,
                SpatialIndices,
                SpikyPow2ScalingFactor,
                SpikyPow3ScalingFactor
            );
        }

        MPI_Send(Densities.data(), batchSize * 2, MPI_FLOAT, 0, 17, MPI_COMM_WORLD);

        MPI_Recv(Densities.data(), numParticles * 2, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(Velocities.data(), numParticles * 2, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);

        // CalculatePressureForce

        MPI_Recv(&targetDensity, 1, MPI_FLOAT, 0, 18, MPI_COMM_WORLD, &status);
        MPI_Recv(&pressureMultiplier, 1, MPI_FLOAT, 0, 19, MPI_COMM_WORLD, &status);
        MPI_Recv(&nearPressureMultiplier, 1, MPI_FLOAT, 0, 20, MPI_COMM_WORLD, &status);
        MPI_Recv(&SpikyPow2DerivativeScalingFactor, 1, MPI_FLOAT, 0, 21, MPI_COMM_WORLD, &status);
        MPI_Recv(&spikyPow3DerivativeScalingFactor, 1, MPI_FLOAT, 0, 22, MPI_COMM_WORLD, &status);

        for (int i = 0; i < batchSize; i++)
        {
            CalculatePressureForce(
                i,numParticles,Densities,PredictedPositions,smoothingRadius,SpatialOffsets,
                SpatialIndices,Velocities,deltaTime,
                targetDensity,pressureMultiplier,nearPressureMultiplier,
                SpikyPow2DerivativeScalingFactor, spikyPow3DerivativeScalingFactor
            );
        }

        MPI_Send(Velocities.data(), batchSize * 2, MPI_FLOAT, 0, 23, MPI_COMM_WORLD);


        // CalculateViscosity

        MPI_Recv(&Poly6ScalingFactor, 1, MPI_FLOAT, 0, 24, MPI_COMM_WORLD, &status);
        MPI_Recv(&viscosityStrength, 1, MPI_FLOAT, 0, 25, MPI_COMM_WORLD, &status);

        for (int i = 0; i < batchSize; i++)
        {
            CalculateViscosity(i, numParticles, PredictedPositions, smoothingRadius, Velocities,
                SpatialOffsets, SpatialIndices, Poly6ScalingFactor,
                viscosityStrength,
                deltaTime);
        }

        MPI_Send(Velocities.data(), batchSize * 2, MPI_FLOAT, 0, 26, MPI_COMM_WORLD);


        // UpdatePositions

        MPI_Recv(&boundsSize, 2, MPI_FLOAT, 0, 27, MPI_COMM_WORLD, &status);
        MPI_Recv(&collisionDamping, 1, MPI_FLOAT, 0, 28, MPI_COMM_WORLD, &status);

        for (int i = 0; i < batchSize; i++)
        {
            UpdatePositions(i, numParticles, Positions, Velocities, deltaTime,
                boundsSize, collisionDamping);
        }

        MPI_Send(Positions.data(), batchSize * 2, MPI_FLOAT, 0, 29, MPI_COMM_WORLD);
        MPI_Send(Velocities.data(), batchSize * 2, MPI_FLOAT, 0, 30, MPI_COMM_WORLD);
    }
}

void MpiWorker::ExternalForces(
    std::vector<Float2>& Velocities,
    std::vector<Float2>& Positions,
    std::vector<Float2>& PredictedPositions,
    float deltaTime,
    int id, float gravity, float currentInteractionInputStrength,
    Float2& interactionInputPoint, float interactionInputRadius)
{
    Velocities[id] += ExternalForces(
        Positions[id], Velocities[id], gravity, currentInteractionInputStrength,
       interactionInputPoint, interactionInputRadius) * deltaTime;
    const float predictionFactor = 1 / 120.0;
    PredictedPositions[id] = Positions[id] + Velocities[id] * predictionFactor;
}

Float2 MpiWorker::ExternalForces(Float2& pos, Float2& velocity, float gravity,
    float currentInteractionInputStrength,
    Float2& interactionInputPoint, float interactionInputRadius)
{
    Float2 gravityAccel = Float2(0, gravity);
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

void MpiWorker::CalculateDensity(int id,
    std::vector<Float2>& PredictedPositions,
    std::vector<Float2>& Densities,
    float smoothingRadius,
    std::vector<ImU32>& SpatialOffsets,
    ImU32 numParticles,
    std::vector<SpatialEntry>& SpatialIndices,
    float SpikyPow2ScalingFactor,
    float SpikyPow3ScalingFactor
)
{
    Float2 pos = PredictedPositions[id];
    Densities[id] = CalculateDensityForPos(
        pos,
        smoothingRadius,
        SpatialOffsets,
        numParticles,
        SpatialIndices,
        PredictedPositions,
        SpikyPow2ScalingFactor,
        SpikyPow3ScalingFactor
    );
}


static const Int2 offsets2D[9] =
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


Float2 MpiWorker::CalculateDensityForPos(Float2 pos,
    float smoothingRadius,
    std::vector<ImU32>& SpatialOffsets,
    ImU32 numParticles,
    std::vector<SpatialEntry>& SpatialIndices,
    std::vector<Float2>& PredictedPositions,
    float SpikyPow2ScalingFactor,
    float SpikyPow3ScalingFactor
)
{
    Int2 originCell = Physics::GetCell2D(pos, smoothingRadius);
    float sqrRadius = smoothingRadius * smoothingRadius;
    float density = 0;
    float nearDensity = 0;
    // Neighbour search
    for (int i = 0; i < 9; i++)
    {
        ImU32 hash = Physics::HashCell2D(originCell + offsets2D[i]);
        ImU32 key = Physics::KeyFromHash(hash, numParticles);
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
            density += DensityKernel(dst, smoothingRadius, SpikyPow2ScalingFactor);
            nearDensity += NearDensityKernel(dst, smoothingRadius, SpikyPow3ScalingFactor);

        }
    }

    return Float2(density, nearDensity);
}

float MpiWorker::DensityKernel(float dst, float radius, float SpikyPow2ScalingFactor)
{
    if (dst < radius)
    {
        float v = radius - dst;
        return v * v * SpikyPow2ScalingFactor;
    }
    return 0;
}

float MpiWorker::NearDensityKernel(float dst, float radius, float SpikyPow3ScalingFactor)
{
    if (dst < radius)
    {
        float v = radius - dst;
        return v * v * v * SpikyPow3ScalingFactor;
    }
    return 0;
}

void MpiWorker::CalculatePressureForce(int id, ImU32 numParticles, std::vector<Float2>& Densities,
    std::vector<Float2>& PredictedPositions,
    float smoothingRadius,
    std::vector<ImU32>& SpatialOffsets,
    std::vector<SpatialEntry>& SpatialIndices,
    std::vector<Float2>& Velocities,
    float deltaTime,
    float targetDensity, float pressureMultiplier, float nearPressureMultiplier,
    float SpikyPow2DerivativeScalingFactor, float SpikyPow3DerivativeScalingFactor
    )
{
    if (id >= numParticles) return;

    float density = Densities[id][0];
    float densityNear = Densities[id][1];
    float pressure = PressureFromDensity(density, targetDensity, pressureMultiplier);
    float nearPressure = NearPressureFromDensity(densityNear, nearPressureMultiplier);
    Float2 pressureForce = 0;

    Float2 pos = PredictedPositions[id];
    Int2 originCell = Physics::GetCell2D(pos, smoothingRadius);
    float sqrRadius = smoothingRadius * smoothingRadius;

    // Neighbour search
    for (int i = 0; i < 9; i++)
    {
        ImU32 hash = Physics::HashCell2D(originCell + offsets2D[i]);
        ImU32 key = Physics::KeyFromHash(hash, numParticles);
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
            float neighbourPressure = PressureFromDensity(neighbourDensity, targetDensity, pressureMultiplier);
            float neighbourNearPressure = NearPressureFromDensity(neighbourNearDensity, nearPressureMultiplier);

            float sharedPressure = (pressure + neighbourPressure) * 0.5;
            float sharedNearPressure = (nearPressure + neighbourNearPressure) * 0.5;

            pressureForce += dirToNeighbour * DensityDerivative(dst, smoothingRadius, SpikyPow2DerivativeScalingFactor) * sharedPressure / neighbourDensity;
            pressureForce += dirToNeighbour * NearDensityDerivative(dst, smoothingRadius, SpikyPow3DerivativeScalingFactor) * sharedNearPressure / neighbourNearDensity;
        }
    }

    Float2 acceleration = pressureForce / density;
    Velocities[id] -= acceleration * deltaTime;
}

float MpiWorker::PressureFromDensity(float density, float targetDensity, float pressureMultiplier)
{
    return (density - targetDensity) * pressureMultiplier;
}

float MpiWorker::NearPressureFromDensity(float nearDensity, float nearPressureMultiplier)
{
    return nearPressureMultiplier * nearDensity;
}

float MpiWorker::DensityDerivative(float dst, float radius, float SpikyPow2DerivativeScalingFactor)
{
    if (dst <= radius)
    {
        float v = radius - dst;
        return -v * SpikyPow2DerivativeScalingFactor;
    }
    return 0;
}

float MpiWorker::NearDensityDerivative(float dst, float radius, float SpikyPow3DerivativeScalingFactor)
{
    if (dst <= radius)
    {
        float v = radius - dst;
        return -v * v * SpikyPow3DerivativeScalingFactor;
    }
    return 0;
}

void MpiWorker::CalculateViscosity(
    int id,
    int numParticles,
    std::vector<Float2>& PredictedPositions,
    float smoothingRadius, std::vector<Float2>& Velocities, std::vector<ImU32>& SpatialOffsets,
    std::vector<SpatialEntry>& SpatialIndices,
    float Poly6ScalingFactor,
    float viscosityStrength,
    float deltaTime
    )
{
    if (id >= numParticles) return;


    Float2 pos = PredictedPositions[id];
    Int2 originCell = Physics::GetCell2D(pos, smoothingRadius);
    float sqrRadius = smoothingRadius * smoothingRadius;

    Float2 viscosityForce = 0;
    Float2 velocity = Velocities[id];

    for (int i = 0; i < 9; i++)
    {
        ImU32 hash = Physics::HashCell2D(originCell + offsets2D[i]);
        ImU32 key = Physics::KeyFromHash(hash, numParticles);
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
            viscosityForce += (neighbourVelocity - velocity) * ViscosityKernel(dst, smoothingRadius, Poly6ScalingFactor);
        }

    }
    Velocities[id] -= viscosityForce * viscosityStrength * deltaTime;
}

float MpiWorker::ViscosityKernel(float dst, float radius, float Poly6ScalingFactor)
{
    if (dst < radius)
    {
        float v = radius * radius - dst * dst;
        return v * v * v * Poly6ScalingFactor;
    }
    return 0;
}

void MpiWorker::UpdatePositions(int id, int numParticles, std::vector<Float2>& Positions,
    std::vector<Float2>& Velocities, float deltaTime, Float2 boundsSize, float collisionDamping)
{
    if (id >= numParticles) return;

    Positions[id] += Velocities[id] * deltaTime;
    HandleCollisions(id, Positions, Velocities, boundsSize, collisionDamping);
}

void MpiWorker::HandleCollisions(ImU32 particleIndex, std::vector<Float2>& Positions,
    std::vector<Float2>& Velocities, Float2 boundsSize, float collisionDamping)
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
