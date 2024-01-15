#pragma once
#include <imgui.h>
#include <math.h>
#include <algorithm>

template<typename T>
struct Vec2 {
    Vec2() : x(0), y(0) {}
    Vec2(T value) : x(value), y(value) {}
    Vec2(T _x, T _y) : x(_x), y(_y) {}
    template<typename Other>
    Vec2(Vec2<Other> vec) : x((T)vec.x), y((T)vec.y) {}

    Vec2<T> operator + (const Vec2<T> other) const {
        return { x + other.x, y + other.y };
    }

    Vec2<T> operator - (const Vec2<T> other) const {
        return { x - other.x, y - other.y };
    }

    Vec2<T> operator * (const Vec2<T> other) const {
        return { x * other.x, y * other.y };
    }

    Vec2<T> operator / (const Vec2<T> other) const {
        return { x / other.x, y / other.y };
    }

    Vec2<T> operator + (T other) const {
        return { x + other, y + other };
    }

    Vec2<T> operator - (T other) const {
        return { x - other, y - other };
    }

    Vec2<T> operator * (T other) const {
        return { x * other, y * other };
    }

    Vec2<T> operator / (T other) const {
        return { x / other, y / other };
    }

    Vec2<T>& operator += (Vec2<T> other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2<T>& operator -= (Vec2<T> other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2<T>& operator *= (Vec2<T> other) {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    Vec2<T>& operator /= (Vec2<T> other) {
        x /= other.x;
        y /= other.y;
        return *this;
    }

    T& operator [](size_t index) {
        return ((T*)this)[index];
    }

    const T& operator [](size_t index) const {
        return ((const T*)this)[index];
    }

    operator ImVec2() const {
        return { (float)x, (float)y };
    }

    T x;
    T y;
};

typedef Vec2<float> Float2;
typedef Vec2<int> Int2;

inline float Dot(Float2 a, Float2 b) {
    return a.x * b.x + a.y * b.y;
}

inline Float2 Floor(Float2 a) {
    return { floorf(a.x), floorf(a.y) };
}

inline Float2 Abs(Float2 a) {
    return { fabsf(a.x), fabsf(a.y) };
}

inline float sign(float value) {
    if (value < 0.0f) {
        return -1.0f;
    }
    else if (value > 0.0f) {
        return 1.0f;
    }
    else {
        return 0.0f;
    }
}

inline float saturate(float x)
{
    return std::max(0.0f, std::min(1.0f, x));
}

inline Int2 ToInt2(ImVec2 vec) {
    return { (int)vec.x, (int)vec.y };
}
