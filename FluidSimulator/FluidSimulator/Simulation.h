#pragma once
#include "Vec2.h"
#include "physics.h"
#include "ParticleSpawner.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <GLFW\glfw3.h>
#include <imgui_impl_glfw.h>
#include <thread>

#define SIMULATION_PARAM_FACTOR 4.0f
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 920
#define RUN_MPI 1

#if RUN_MPI
#include <mpi.h>
#else
#include "ThreadPool.h"
#endif

struct Simulation
{
    Simulation()
    {
        SetDefaultParams();
    }

    void SetDefaultParams() {
        physics.interactionInputPoint = 0.0f;
        physics.interactionInputRadius = 50.0f;
        physics.interactionInputStrength = 1000.0f;
        physics.gravity = 40.0f * SIMULATION_PARAM_FACTOR;
        physics.collisionDamping = 0.4f;
        physics.smoothingRadius = 2.75f * SIMULATION_PARAM_FACTOR;
        physics.targetDensity = 6.f * SIMULATION_PARAM_FACTOR;
        physics.pressureMultiplier = 30.0f * SIMULATION_PARAM_FACTOR;
        physics.nearPressureMultiplier = 1.75f * SIMULATION_PARAM_FACTOR;
        physics.viscosityStrength = 0.075f * SIMULATION_PARAM_FACTOR;
    }

    void Start()
    {
#if !RUN_MPI
        pool.~ThreadPool(); // reset thread pool
        const auto available_thread_count = (std::thread::hardware_concurrency() - 1); // minus current main thread
        new(&pool) ThreadPool(available_thread_count); // c++...
#endif
        //Debug.Log("Controls: Space = Play/Pause, R = Reset, LMB = Attract, RMB = Repel");
        frameIndex = 0;
        isPaused = false;
        pauseNextFrame = false;

        auto spawnData = spawner.GetSpawnData();

        physics.numParticles = spawnData.positions.size();
        // Create buffers
        physics.ResizeBuffers();

        // Set buffer data
        SetInitialBufferData(spawnData);

        /*physics.interactionInputPoint = physicsParameters.interactionInputPoint;
        physics.interactionInputRadius = physicsParameters.interactionInputRadius;
        physics.interactionInputStrength  = physicsParameters.interactionInputStrength;
        physics.gravity = physicsParameters.gravity;
        physics.collisionDamping = physicsParameters.collisionDamping;
        physics.smoothingRadius = physicsParameters.smoothingRadius;
        physics.targetDensity = physicsParameters.targetDensity;
        physics.pressureMultiplier = physicsParameters.pressureMultiplier;
        physics.nearPressureMultiplier = physicsParameters.nearPressureMultiplier;
        physics.viscosityStrength = physicsParameters.viscosityStrength;*/
        physics.boundsSize = { SCREEN_WIDTH, SCREEN_HEIGHT };
        // Init display
        // display.Init(this);
    }

    void Update(GLFWwindow* window, float currentDeltaTime)
    {
        currentDeltaTime = std::min(0.02f, currentDeltaTime);
        physics.deltaTime = currentDeltaTime;

        // Run simulation if not in fixed timestep mode
        // (skip running for first few frames as deltaTime can be disproportionaly large)
        //MoveParticles();
        // if (frameIndex > 10)
        // {

        static bool was_w_pressed = false;
        bool is_w = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        bool is_a = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
        bool is_s = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        bool is_d = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

        //if (!was_w_pressed && is_w) {
            RunSimulationFrame(window, physics.deltaTime);
            was_w_pressed = true;
        //}

        if (!is_w) {
            was_w_pressed = false;
        }
        // }

        // if (pauseNextFrame)
        // {
        //     isPaused = true;
        //     pauseNextFrame = false;
        // }

        // HandleInput();
    }

    void RunSimulationFrame(GLFWwindow* window, float frameTime)
    {
        if (!isPaused)
        {
            float timeStep = frameTime / iterationsPerFrame * timeScale;

            UpdateSettings(window, timeStep);

            for (int i = 0; i < iterationsPerFrame; i++)
            {
                RunSimulationStep();
            }
        }
    }

    void RunSimulationStep()
    {
        //run tasks
#if RUN_MPI
        RunSimulationStepMPI();
#else
        RunSimulationStepMultithreaded();
#endif
    }

#if !RUN_MPI
    void RunThreadPoolBatch(std::function<void(int)> fun)
    {

        int batchSize = physics.numParticles / pool.size();

        for (int i = 0; i < physics.numParticles; i += batchSize)
        {
            pool.enqueueFunction([this, i, batchSize, fun]() {
                for (int index = i; index < i + batchSize && index < physics.numParticles; index++)
                {
                    fun(index);
                }
            });
        }
        pool.waitUntilAllThreadsWait();
    }

    void RunSimulationStepMultithreaded()
    {
        RunThreadPoolBatch([this](int i) { physics.ExternalForces(i); });
        // RunThreadPoolBatch([this](int i) { physics.UpdateSpatialHash(i); }); // concurrency issues because of the spatial hash...
        for (int i = 0; i < physics.numParticles; i++)
        {
            physics.UpdateSpatialHash(i);
        }

        physics.GpuSortAndCalculateOffsets();

        RunThreadPoolBatch([this](int i) { physics.CalculateDensity(i); });
        RunThreadPoolBatch([this](int i) { physics.CalculatePressureForce(i); });
        RunThreadPoolBatch([this](int i) { physics.CalculateViscosity(i); });
        RunThreadPoolBatch([this](int i) { physics.UpdatePositions(i); });
    }
#else
    void RunSimulationStepMPI()
    {
        ExternalForcesMPI();

        for (int i = 0; i < physics.numParticles; i++)
        {
            physics.UpdateSpatialHash(i);
        }

        physics.GpuSortAndCalculateOffsets();

        CalculateDensityMPI();

        CalculatePressureForceMPI();

        CalculateViscosityMPI();
        //for (int i = 0; i < physics.numParticles; i++)
        //{
        //    physics.CalculateViscosity(i);
        //}

        for (int i = 0; i < physics.numParticles; i++)
        {
            physics.UpdatePositions(i);
        }
    }

    void ExternalForcesMPI()
    {
        int chunk_size = physics.numParticles / mpiWorkersCount;
        for (int i = 0; i < mpiWorkersCount; i++)
        {
            int start = i * chunk_size;
            int end = (i == mpiWorkersCount - 1) ? physics.numParticles : (i + 1) * chunk_size;
            int actualChunkSize = end - start;

            MPI_Ssend(&actualChunkSize, 1, MPI_INT, i + 1, 1, MPI_COMM_WORLD);
            MPI_Ssend(physics.Velocities.data() + start, actualChunkSize * 2, MPI_FLOAT, i + 1, 2, MPI_COMM_WORLD);
            MPI_Ssend(physics.Positions.data() + start, actualChunkSize * 2, MPI_FLOAT, i + 1, 3, MPI_COMM_WORLD);
            MPI_Ssend(&physics.deltaTime, 1, MPI_FLOAT, i + 1, 4, MPI_COMM_WORLD);
            MPI_Ssend(&physics.gravity, 1, MPI_FLOAT, i + 1, 5, MPI_COMM_WORLD);
            MPI_Ssend(&physics.currentInteractionInputStrength, 1, MPI_FLOAT, i + 1, 6, MPI_COMM_WORLD);
            MPI_Ssend(&physics.interactionInputPoint, 2, MPI_FLOAT, i + 1, 7, MPI_COMM_WORLD);
            MPI_Ssend(&physics.interactionInputRadius, 1, MPI_FLOAT, i + 1, 8, MPI_COMM_WORLD);
        }

        MPI_Status status;
        for (int i = 0; i < mpiWorkersCount; i++)
        {
            int start = i * chunk_size;
            int end = (i == mpiWorkersCount - 1) ? physics.numParticles : (i + 1) * chunk_size;
            int actualChunkSize = end - start;

            MPI_Recv(physics.Velocities.data() + start, actualChunkSize * 2, MPI_FLOAT, i + 1, 9, MPI_COMM_WORLD, &status);
            MPI_Recv(physics.PredictedPositions.data() + start, actualChunkSize * 2, MPI_FLOAT, i + 1, 10, MPI_COMM_WORLD, &status);
        }
    }

    void CalculateDensityMPI()
    {
        int chunk_size = physics.numParticles / mpiWorkersCount;
        for (int i = 0; i < mpiWorkersCount; i++)
        {
            int start = i * chunk_size;
            int end = (i == mpiWorkersCount - 1) ? physics.numParticles : (i + 1) * chunk_size;
            int actualChunkSize = end - start;

            MPI_Ssend(&physics.smoothingRadius, 1, MPI_FLOAT, i + 1, 11, MPI_COMM_WORLD);
            MPI_Ssend(&physics.numParticles, 1, MPI_UINT32_T, i + 1, 12, MPI_COMM_WORLD);
            MPI_Ssend(physics.SpatialOffsets.data(), physics.numParticles, MPI_UINT32_T, i + 1, 13, MPI_COMM_WORLD);
            MPI_Ssend(physics.SpatialIndices.data(), physics.numParticles * 3, MPI_UINT32_T, i + 1, 14, MPI_COMM_WORLD);
            MPI_Ssend(&physics.SpikyPow2ScalingFactor, 1, MPI_FLOAT, i + 1, 15, MPI_COMM_WORLD);
            MPI_Ssend(&physics.SpikyPow3ScalingFactor, 1, MPI_FLOAT, i + 1, 16, MPI_COMM_WORLD);
        }

        MPI_Status status;
        for (int i = 0; i < mpiWorkersCount; i++)
        {
            int start = i * chunk_size;
            int end = (i == mpiWorkersCount - 1) ? physics.numParticles : (i + 1) * chunk_size;
            int actualChunkSize = end - start;

            MPI_Recv(physics.Densities.data() + start, actualChunkSize * 2, MPI_FLOAT, i + 1, 17, MPI_COMM_WORLD, &status);
        }
    }

    void CalculatePressureForceMPI()
    {
        int chunk_size = physics.numParticles / mpiWorkersCount;
        for (int i = 0; i < mpiWorkersCount; i++)
        {
            int start = i * chunk_size;
            int end = (i == mpiWorkersCount - 1) ? physics.numParticles : (i + 1) * chunk_size;
            int actualChunkSize = end - start;

            MPI_Ssend(&physics.targetDensity, 1, MPI_FLOAT, i + 1, 18, MPI_COMM_WORLD);
            MPI_Ssend(&physics.pressureMultiplier, 1, MPI_FLOAT, i + 1, 19, MPI_COMM_WORLD);
            MPI_Ssend(&physics.nearPressureMultiplier, 1, MPI_FLOAT, i + 1, 20, MPI_COMM_WORLD);
            MPI_Ssend(&physics.SpikyPow2DerivativeScalingFactor, 1, MPI_FLOAT, i + 1, 21, MPI_COMM_WORLD);
            MPI_Ssend(&physics.SpikyPow3DerivativeScalingFactor, 1, MPI_FLOAT, i + 1, 22, MPI_COMM_WORLD);
        }

        MPI_Status status;
        for (int i = 0; i < mpiWorkersCount; i++)
        {
            int start = i * chunk_size;
            int end = (i == mpiWorkersCount - 1) ? physics.numParticles : (i + 1) * chunk_size;
            int actualChunkSize = end - start;

            MPI_Recv(physics.Velocities.data() + start, actualChunkSize * 2, MPI_FLOAT, i + 1, 23, MPI_COMM_WORLD, &status);
        }
    }

    void CalculateViscosityMPI()
    {
        int chunk_size = physics.numParticles / mpiWorkersCount;
        for (int i = 0; i < mpiWorkersCount; i++)
        {
            int start = i * chunk_size;
            int end = (i == mpiWorkersCount - 1) ? physics.numParticles : (i + 1) * chunk_size;
            int actualChunkSize = end - start;

            MPI_Ssend(&physics.Poly6ScalingFactor, 1, MPI_FLOAT, i + 1, 24, MPI_COMM_WORLD);
            MPI_Ssend(&physics.viscosityStrength, 1, MPI_FLOAT, i + 1, 25, MPI_COMM_WORLD);
        }

        MPI_Status status;
        for (int i = 0; i < mpiWorkersCount; i++)
        {
            int start = i * chunk_size;
            int end = (i == mpiWorkersCount - 1) ? physics.numParticles : (i + 1) * chunk_size;
            int actualChunkSize = end - start;

            MPI_Recv(physics.Velocities.data() + start, actualChunkSize * 2, MPI_FLOAT, i + 1, 26, MPI_COMM_WORLD, &status);
        }
    }
#endif

    void UpdateSettings(GLFWwindow* window, float deltaTime)
    {
        
        physics.Poly6ScalingFactor = 4 / (M_PI * pow(physics.smoothingRadius, 8));
        physics.SpikyPow3ScalingFactor = 10 / (M_PI * pow(physics.smoothingRadius, 5));
        physics.SpikyPow2ScalingFactor = 6 / (M_PI * pow(physics.smoothingRadius, 4));
        physics.SpikyPow3DerivativeScalingFactor = 30 / (pow(physics.smoothingRadius, 5) * M_PI);
        physics.SpikyPow2DerivativeScalingFactor = 12 / (pow(physics.smoothingRadius, 4) * M_PI);

        // Mouse interaction settings:
        Float2 mousePos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };
        bool isPullInteraction = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        bool isPushInteraction = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        float currInteractStrength = 0;
        if (isPushInteraction || isPullInteraction)
        {
            currInteractStrength = isPushInteraction ? -physics.interactionInputStrength : physics.interactionInputStrength;
        }

        physics.interactionInputPoint = mousePos;
        physics.currentInteractionInputStrength = currInteractStrength;
    }

    void SetInitialBufferData(ParticleSpawnData spawnData)
    {
        physics.Positions = spawnData.positions;
        physics.PredictedPositions = spawnData.positions;
        physics.Velocities = spawnData.velocities;
    }

    void HandleInput()
    {


        //GLFW!!!
        //if (Input.GetKeyDown(KeyCode.Space))
        //{
        //    isPaused = !isPaused;
        //}
        //if (Input.GetKeyDown(KeyCode.RightArrow))
        //{
        //    isPaused = false;
        //    pauseNextFrame = true;
        //}

        //if (Input.GetKeyDown(KeyCode.R))
        //{
        //    isPaused = true;
        //    // Reset positions, the run single frame to get density etc (for debug purposes) and then reset positions again
        //    SetInitialBufferData(spawnData);
        //    RunSimulationStep();
        //    SetInitialBuffer        System.Array.Copy(spawnData.positions, allPoints, spawnData.positions.Length);
//Data(spawnData);
        //}
    }

    void OnDrawGizmos()
    {
        /*Gizmos.color = new Color(0, 1, 0, 0.4f);
        Gizmos.DrawWireCube(Vector2.zero, boundsSize);
        Gizmos.DrawWireCube(obstacleCentre, obstacleSize);

        if (Application.isPlaying)
        {
            Vector2 mousePos = Camera.main.ScreenToWorldPoint(Input.mousePosition);
            bool isPullInteraction = Input.GetMouseButton(0);
            bool isPushInteraction = Input.GetMouseButton(1);
            bool isInteracting = isPullInteraction || isPushInteraction;
            if (isInteracting)
            {
                Gizmos.color = isPullInteraction ? Color.green : Color.red;
                Gizmos.DrawWireSphere(mousePos, interactionRadius);
            }
        }*/

    }

    void MoveParticles() 
    {
        for (auto& particle : physics.Positions)
        {
            particle.x += 1;
        }
    }

    Physics physics;
    ParticleSpawner spawner;
#if !RUN_MPI
    ThreadPool pool;
#else
    int mpiWorkersCount = 0;
#endif

    float timeScale = 1;
    int iterationsPerFrame = 1;

     //ParticleDisplay2D display; ????

    // Buffers

    // State
    bool isPaused;
    bool pauseNextFrame;

    int frameIndex;
};

