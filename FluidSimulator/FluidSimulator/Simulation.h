#pragma once
#include "Vec2.h"
#include "physics.h"
#include "ParticleSpawner.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <GLFW\glfw3.h>

struct Simulation
{
    Simulation()
    {
    }

    void Start()
    {
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

        const float FACTOR = 2.0f;

        physics.interactionInputPoint = 0.0f;
        physics.interactionInputRadius = 0.0f;
        physics.interactionInputStrength  = 0.0f;
        physics.gravity = 50.0f * FACTOR;
        physics.collisionDamping = 0.95f;
        physics.smoothingRadius = 15.f * FACTOR;
        physics.targetDensity = 6.0f * FACTOR;
        physics.pressureMultiplier = 30.0f * FACTOR;
        physics.nearPressureMultiplier = 1.75f * FACTOR;
        physics.viscosityStrength = 0.075f * FACTOR;
        physics.boundsSize = { 1280, 720 };

        // Init display
        // display.Init(this);
    }

    void Update(GLFWwindow* window, float currentDeltaTime)
    {
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
            RunSimulationFrame(physics.deltaTime);
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

        for (int i = 0; i < physics.numParticles; i++) {
            physics.UpdatePositions(i);
        }
    }

    void UpdateSettings(float deltaTime)
    {
        
        physics.Poly6ScalingFactor = 4 / (M_PI * pow(physics.smoothingRadius, 8));
        physics.SpikyPow3ScalingFactor = 10 / (M_PI * pow(physics.smoothingRadius, 5));
        physics.SpikyPow2ScalingFactor = 6 / (M_PI * pow(physics.smoothingRadius, 4));
        physics.SpikyPow3DerivativeScalingFactor = 30 / (pow(physics.smoothingRadius, 5) * M_PI);
        physics.SpikyPow2DerivativeScalingFactor = 12 / (pow(physics.smoothingRadius, 4) * M_PI);

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
    int iterationsPerFrame = 1;

     //ParticleDisplay2D display; ????

    // Buffers

    // State
    bool isPaused;
    bool pauseNextFrame;

    int frameIndex;
};

