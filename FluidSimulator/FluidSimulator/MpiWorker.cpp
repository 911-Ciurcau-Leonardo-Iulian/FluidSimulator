#include "MpiWorker.h"
#include <mpi.h>
#include <iostream>
#include <thread>

void MpiWorker::run()
{
    MPI_Status status;
    int batchSize, numParticles;
    while (true)
    {
        MPI_Recv(&batchSize, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&numParticles, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
    }
}
