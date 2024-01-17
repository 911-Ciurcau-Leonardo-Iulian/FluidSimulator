#pragma once
#include <imgui.h>
#include <math.h>
#include <algorithm>

template<typename T>
struct Vec4 {
    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(T value) : x(value), y(value), z(value), w(value) {}
    Vec4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}
    template<typename Other>
    Vec4(Vec4<Other> vec) : x((T)vec.x), y((T)vec.y), z((T)vec.z), w((T)vec.w) {}

    Vec4<T> operator + (const Vec4<T> other) const {
        return { x + other.x, y + other.y, z + other.z, w + other.w };
    }

    Vec4<T> operator - (const Vec4<T> other) const {
        return { x - other.x, y - other.y, z - other.z, w - other.w };
    }

    Vec4<T> operator * (const Vec4<T> other) const {
        return { x * other.x, y * other.y, z * other.z, w / other.w };
    }

    Vec4<T> operator / (const Vec4<T> other) const {
        return { x / other.x, y / other.y, z / other.z, w / other.w };
    }

    Vec4<T> operator + (T other) const {
        return { x + other, y + other, z + other, w + other };
    }

    Vec4<T> operator - (T other) const {
        return { x - other, y - other, z - other, w - other };
    }

    Vec4<T> operator * (T other) const {
        return { x * other, y * other, z * other, w * other };
    }

    Vec4<T> operator / (T other) const {
        return { x / other, y / other, z / other, w / other };
    }

    Vec4<T>& operator += (Vec4<T> other) {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    Vec4<T>& operator -= (Vec4<T> other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    Vec4<T>& operator *= (Vec4<T> other) {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
        return *this;
    }

    Vec4<T>& operator /= (Vec4<T> other) {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        w /= other.w;
        return *this;
    }

    T& operator [](size_t index) {
        return ((T*)this)[index];
    }

    const T& operator [](size_t index) const {
        return ((const T*)this)[index];
    }

    T x;
    T y;
    T z;
    T w;
};

typedef Vec4<float> Float4;
typedef Vec4<int> Int4;
typedef Vec4<unsigned char> UChar4;

inline float Dot(Float4 a, Float4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
