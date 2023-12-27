#include "particle.h"

Particle::Particle(const ImVec2& position, const float& radius, const ImVec2& velocity, const ImVec2& acceleration)
{
    this->position = position;
    this->radius = radius;
    this->velocity = velocity;
    this->acceleration = acceleration;
}


void Particle::draw() 
{
    ImGui::GetWindowDrawList()->AddCircle(position, radius, color);
}