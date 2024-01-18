#include "MpiWorker2.h"
#include <mpi.h>

void MpiWorker::Run()
{
    MPI_Status status;
    unsigned int particle_count;
    MPI_Recv(&particle_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

    physics.numParticles = particle_count;
    physics.ResizeBuffers();

    while (true) {
        size_t parameter_size = offsetof(Physics, Positions);

        MPI_Recv(this, parameter_size / sizeof(int), MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        MpiWorkerRange range;
        MPI_Recv(&range, 2, MPI_UINT32_T, 0, 0, MPI_COMM_WORLD, &status);

        MPI_Recv(physics.Velocities.data(), particle_count * 2, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(physics.PredictedPositions.data(), particle_count * 2, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(physics.SpatialIndices.data(), particle_count * 3, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(physics.SpatialOffsets.data(), particle_count, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        // Calculate density
        for (unsigned int index = range.start; index < range.end; index++) {
            physics.CalculateDensity(index);
        }

        // Send back the densities
        MPI_Ssend(physics.Densities.data() + range.start, (range.end - range.start) * 2, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);

        MPI_Recv(physics.Densities.data(), particle_count * 2, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);

        for (unsigned int index = range.start; index < range.end; index++) {
            physics.CalculatePressureForce(index);
        }

        // Send back the velocities
        MPI_Ssend(physics.Velocities.data() + range.start, (range.end - range.start) * 2, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);

        MPI_Recv(physics.Velocities.data(), particle_count * 2, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);

        for (unsigned int index = range.start; index < range.end; index++) {
            physics.CalculateViscosity(index);
        }

        // Send back again the velocities
        MPI_Ssend(physics.Velocities.data() + range.start, (range.end - range.start) * 2, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }
}
