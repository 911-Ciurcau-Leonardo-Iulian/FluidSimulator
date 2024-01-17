#include "MpiWorker.h"

void MpiWorker::run()
{
    MPI_Status status;
    int batchSize, numParticles;
    std::vector<Float2> Velocities, Positions, PredictedPositions;
    float deltaTime, gravity, currentInteractionInputStrength, interactionInputRadius;
    Float2 interactionInputPoint;

    while (true)
    {
        MPI_Recv(&batchSize, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        Velocities.resize(batchSize);
        MPI_Recv(Velocities.data(), batchSize * 2, MPI_FLOAT, 0, 2, MPI_COMM_WORLD, &status);
        Positions.resize(batchSize);
        MPI_Recv(Positions.data(), batchSize * 2, MPI_FLOAT, 0, 3, MPI_COMM_WORLD, &status);
        MPI_Recv(&deltaTime , 1, MPI_FLOAT, 0, 4, MPI_COMM_WORLD, &status);
        MPI_Recv(&gravity , 1, MPI_FLOAT, 0, 5, MPI_COMM_WORLD, &status);
        MPI_Recv(&currentInteractionInputStrength, 1, MPI_FLOAT, 0, 6, MPI_COMM_WORLD, &status);
        MPI_Recv(&interactionInputPoint, 2, MPI_FLOAT, 0, 7, MPI_COMM_WORLD, &status);
        MPI_Recv(&interactionInputRadius, 1, MPI_FLOAT, 0, 8, MPI_COMM_WORLD, &status);

        PredictedPositions.resize(batchSize);

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

Float2 MpiWorker::ExternalForces(Float2& pos, Float2& velocity, float gravity, float currentInteractionInputStrength,
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
