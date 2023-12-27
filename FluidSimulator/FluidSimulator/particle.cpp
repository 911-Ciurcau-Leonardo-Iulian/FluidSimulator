#include "particle.h"
#include <algorithm>

float Particle::radius;

Particle::Particle(const ImVec2& position, const ImVec2& velocity, const ImVec2& acceleration)
{
    this->position = position;
    this->velocity = velocity;
    this->acceleration = acceleration;
    Particle::radius = std::min(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()) / 64.0;
}


void Particle::draw() 
{
    ImGui::GetWindowDrawList()->AddCircleFilled(position, Particle::radius, color);
}