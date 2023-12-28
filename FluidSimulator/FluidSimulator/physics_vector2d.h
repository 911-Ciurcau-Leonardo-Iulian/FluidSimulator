template<typename T>
class PhysicsVector2D {
public:
    T x, y;

    PhysicsVector2D();
    PhysicsVector2D(T scalar);
    PhysicsVector2D(T x, T y);

    static PhysicsVector2D floor(PhysicsVector2D vec)
    {
        return PhysicsVector2D(floor(vec.x), floor(vec.y));
    }

    static PhysicsVector2D abs(PhysicsVector2D vec)
    {
        return PhysicsVector2D(abs(vec.x), abs(vec.y));
    }

    static T& dot(PhysicsVector2D a, PhysicsVector2D b)
    {
        return a.x * b.x + a.y * b.y;
    }

    PhysicsVector2D operator+(PhysicsVector2D const& vec) 
    {
        return PhysicsVector2D(x + vec.x, y + vec.y);
    }

    PhysicsVector2D& operator+=(PhysicsVector2D const& vec) const
    {
        x += vec.x;
        y += vec.y;

        return *this;
    }

    PhysicsVector2D operator-(PhysicsVector2D const& vec) 
    {
        return PhysicsVector2D(x - vec.x, y - vec.y);
    }

    PhysicsVector2D operator-(PhysicsVector2D const& vec) const
    {
        return PhysicsVector2D(x - vec.x, y - vec.y);
    }

    PhysicsVector2D& operator-=(PhysicsVector2D const& vec) const
    {
        x -= vec.x;
        y -= vec.y;

        return *this;
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
        
        default:
            throw std::runtime_error("Index out of bounds for PhysicsVector2D");
            break;
        }
    }
};

template <typename T> T sign(T val) {
    return (T(0) < val) - (val < T(0));
}

template <typename T> T  saturate(T x)
{
  return max(0, min(1, x));
}