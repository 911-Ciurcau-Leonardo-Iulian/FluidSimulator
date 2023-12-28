template<typename T>
class PhysicsVector2D {
public:
    T x, y;

    PhysicsVector2D();
    PhysicsVector2D(T x, T y);

    static PhysicsVector2D floor(PhysicsVector2D vec)
    {
        return PhysicsVector2D(floor(vec.x), floor(vec.y));
    }

    PhysicsVector2D operator+(PhysicsVector2D const& vec) 
    {
        return PhysicsVector2D(x + vec.x, y + vec.y);
    }

    PhysicsVector2D operator-(PhysicsVector2D const& vec) 
    {
        return PhysicsVector2D(x - vec.x, y - vec.y);
    }

    PhysicsVector2D operator*(T const& scalar) const
    {
        return PhysicsVector2D(x * scalar, y * scalar);
    }

    PhysicsVector2D operator/(T& scalar) const
    {
        return PhysicsVector2D(x / scalar, y / scalar);
    }

    operator PhysicsVector2D<int>&() const 
    {
        return PhysicsVector2D<int>(x, y);
    }
};