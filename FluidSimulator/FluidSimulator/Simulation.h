#pragma once
#include "Vec2.h"
#include "physics.h"
#include "ParticleSpawner.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

struct Simulation
{
    Simulation()
    {
    }

    void Start()
    {
        //Debug.Log("Controls: Space = Play/Pause, R = Reset, LMB = Attract, RMB = Repel");

        float deltaTime = 1 / 60.f;

        frameIndex = 0;
        isPaused = false;
        pauseNextFrame = false;

        auto spawnData = spawner.GetSpawnData();

        physics.numParticles = spawnData.positions.size();
        // Create buffers
        physics.ResizeBuffers();

        // Set buffer data
        SetInitialBufferData(spawnData);

        // Init display
        // display.Init(this);
    }

    void Update()
    {
        // Run simulation if not in fixed timestep mode
        // (skip running for first few frames as deltaTime can be disproportionaly large)
        //MoveParticles();
        // if (frameIndex > 10)
        // {
             RunSimulationFrame(deltaTime);
        // }

        // if (pauseNextFrame)
        // {
        //     isPaused = true;
        //     pauseNextFrame = false;
        // }

        // HandleInput();
    }

    void RunSimulationFrame(float frameTime)
    {
        if (!isPaused)
        {
            float timeStep = frameTime / iterationsPerFrame * timeScale;

            UpdateSettings(timeStep);

            for (int i = 0; i < iterationsPerFrame; i++)
            {
                RunSimulationStep();
            }
        }
    }

    void RunSimulationStep()
    {
        std::cout << "RUN SIMULATION STEP\n\n";
        //run tasks
        for (int i = 0; i < physics.numParticles; i++)
        {
            physics.ExternalForces(i);
        }

        for (int i = 0; i < physics.numParticles; i++)
        {
            physics.UpdateSpatialHash(i);
        }

        physics.GpuSortAndCalculateOffsets();

        for (int i = 0; i < physics.numParticles; i++)
        {
            physics.CalculateDensity(i);
        }

        for (int i = 0; i < physics.numParticles; i++)
        {
            physics.CalculatePressureForce(i);
        }

        for (int i = 0; i < physics.numParticles; i++)
        {
            physics.CalculateViscosity(i);
        }
    }

    void UpdateSettings(float deltaTime)
    {
        
        physics.Poly6ScalingFactor = 4 / (M_PI * pow(smoothingRadius, 8));
        physics.SpikyPow3ScalingFactor = 10 / (M_PI * pow(smoothingRadius, 5));
        physics.SpikyPow2ScalingFactor = 6 / (M_PI * pow(smoothingRadius, 4));
        physics.SpikyPow3DerivativeScalingFactor = 30 / (pow(smoothingRadius, 5) * M_PI);
        physics.SpikyPow2DerivativeScalingFactor = 12 / (pow(smoothingRadius, 4) * M_PI);

        // Mouse interaction settings:
        /*Float2 mousePos = Camera.main.ScreenToWorldPoint(Input.mousePosition);
        bool isPullInteraction = Input.GetMouseButton(0);
        bool isPushInteraction = Input.GetMouseButton(1);
        float currInteractStrength = 0;
        if (isPushInteraction || isPullInteraction)
        {
            currInteractStrength = isPushInteraction ? -interactionStrength : interactionStrength;
        }*/

        /*physics.interactionInputPoint = mousePos;
        physics.interactionInputStrength = currInteractStrength;
        physics.interactionInputRadius = interactionRadius;*/
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

    float timeScale = 1;
    bool fixedTimeStep;
    int iterationsPerFrame = 1;
    float gravity;
    float collisionDamping = 0.95f;
    float smoothingRadius = 2;
    float targetDensity;
    float pressureMultiplier;
    float nearPressureMultiplier;
    float viscosityStrength;
    Float2 boundsSize;
    Float2 obstacleSize;
    Float2 obstacleCentre;

    float interactionRadius;
    float interactionStrength;

     //ParticleDisplay2D display; ????

    // Buffers

    // State
    bool isPaused;
    bool pauseNextFrame;

    int frameIndex;
    float deltaTime;
};

