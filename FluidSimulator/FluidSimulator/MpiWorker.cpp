#include "MpiWorker.h"
#include <mpi.h>
#include <iostream>
#include <thread>
#include <vector>
#include "Vec2.h"

void MpiWorker::run()
{
    MPI_Status status;
    int batchSize, numParticles;
    std::vector<Float2> Velocities, Positions;
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
    }
}
