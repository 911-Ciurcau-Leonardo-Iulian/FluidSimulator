#include <stdexcept>

template<typename T>
class PhysicsVector3D {
public:
    T x, y, z;

    PhysicsVector3D();
    PhysicsVector3D(T x, T y, T z);

    T& operator[](int index)
    {
        return switch (index)
        {
        case 0:
            x;
            break;
        
        case 1:
            y;
            break;

        case 2:
            z;
            break;
        
        default:
            throw std::runtime_error("Index out of bounds for PhysicsVector3D");
            break;
        }
    }
};