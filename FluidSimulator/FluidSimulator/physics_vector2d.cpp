#include "physics_vector2d.h"

template<typename T>
PhysicsVector2D<T>::PhysicsVector2D() : x(0), y(0)
{}

template<typename T>
PhysicsVector2D<T>::PhysicsVector2D(T x, T y) : x(x), y(y)
{}
