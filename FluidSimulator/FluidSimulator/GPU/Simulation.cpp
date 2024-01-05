#include "Simulation.h"
#include "glad.h"
#include <iostream>

extern "C" {
    _declspec(dllexport) unsigned int NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#define SHADER_BASE_LOCATION "C:\\Users\\Andrei\\Documents\\Facultate\\Parallel and Distributed Programming\\FluidSimulator\\FluidSimulator\\FluidSimulator\\GPU\\Shaders\\"
#define SHADER_LOCATION(name) SHADER_BASE_LOCATION #name

Simulation::Simulation()
{
    if (!gladLoadGL()) {
        std::cout << "Glad initialization failed";
        abort();
    }
    simulation_compute = ComputeShader(SHADER_LOCATION(simulation_early.comp), 1, 1, 1);
}
