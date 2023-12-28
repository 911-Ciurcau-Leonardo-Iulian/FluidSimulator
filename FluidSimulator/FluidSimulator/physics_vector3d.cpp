#include "physics_vector3d.h"

template<typename T>
PhysicsVector3D<T>::PhysicsVector3D() : x(0), y(0), z(0)
{}

template<typename T>
PhysicsVector3D<T>::PhysicsVector3D(T x, T y, T z) : x(x), y(y), z(z)
{}