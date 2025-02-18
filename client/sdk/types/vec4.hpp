#pragma once

#include <vector>
#pragma once
#define M_PI 3.14159265358979323846
#include <iostream>
#include <cmath>

template <typename T>
class vec4 {
public:
    T x;
    T y;
    T z;
    T w;

    vec4();
    vec4(T A, T B, T C, T D);

    T distance(const vec4<T>& other) const;
    vec4 multiply(const std::vector<float>& matrix) const;
    vec4 multiply(double* matrix) const;
    vec4 Multiply(const std::vector<float>& Matrix) const;
    vec4 operator-(const vec4<T>& other) const;
    vec4 operator+(const vec4<T>& other) const;
};

template <typename T>
vec4<T>::vec4() : x(0), y(0), z(0), w(0) {}

template <typename T>
vec4<T>::vec4(T A, T B, T C, T D) : x(A), y(B), z(C), w(D) {}

template <typename T>
T vec4<T>::distance(const vec4<T>& other) const {
    return sqrt(pow(x - other.x, 2.0) + pow(y - other.y, 2.0) + pow(z - other.z, 2.0) + pow(w - other.w, 2.0));
}

template <typename T>
vec4<T> vec4<T>::Multiply(const std::vector<float>& Matrix) const
{
    /* return Vector4
     (
         static_cast<float>(X) * Matrix[0] + static_cast<float>(Y) * Matrix[4] + static_cast<float>(Z) * Matrix[8] + static_cast<float>(W) * Matrix[12],
         static_cast<float>(X) * Matrix[1] + static_cast<float>(Y) * Matrix[5] + static_cast<float>(Z) * Matrix[9] + static_cast<float>(W) * Matrix[13],
         static_cast<float>(X) * Matrix[2] + static_cast<float>(Y) * Matrix[6] + static_cast<float>(Z) * Matrix[10] + static_cast<float>(W) * Matrix[14],
         static_cast<float>(X) * Matrix[3] + static_cast<float>(Y) * Matrix[7] + static_cast<float>(Z) * Matrix[11] + static_cast<float>(W) * Matrix[15]
     );*/
    return vec4(
        static_cast<float>(x) * Matrix[0] + static_cast<float>(y) * Matrix[4] + static_cast<float>(z) * Matrix[8] + static_cast<float>(w) * Matrix[12],
        static_cast<float>(x) * Matrix[1] + static_cast<float>(y) * Matrix[5] + static_cast<float>(z) * Matrix[9] + static_cast<float>(w) * Matrix[13],
        static_cast<float>(x) * Matrix[2] + static_cast<float>(y) * Matrix[6] + static_cast<float>(z) * Matrix[10] + static_cast<float>(w) * Matrix[14],
        static_cast<float>(x) * Matrix[3] + static_cast<float>(y) * Matrix[7] + static_cast<float>(z) * Matrix[11] + static_cast<float>(w) * Matrix[15]
    );
}


template <typename T>
vec4<T> vec4<T>::multiply(const std::vector<float>& matrix) const
{
    /* return Vector4
     (
         static_cast<float>(
         ) * Matrix[0] + static_cast<float>(y) * Matrix[4] + static_cast<float>(z) * Matrix[8] + static_cast<float>(w) * Matrix[12],
         static_cast<float>(x) * Matrix[1] + static_cast<float>(y) * Matrix[5] + static_cast<float>(z) * Matrix[9] + static_cast<float>(w) * Matrix[13],
         static_cast<float>(x) * Matrix[2] + static_cast<float>(y) * Matrix[6] + static_cast<float>(z) * Matrix[10] + static_cast<float>(w) * Matrix[14],
         static_cast<float>(x) * Matrix[3] + static_cast<float>(y) * Matrix[7] + static_cast<float>(z) * Matrix[11] + static_cast<float>(w) * Matrix[15]
     );*/
    return vec4(
        static_cast<float>(x) * matrix[0] + static_cast<float>(y) * matrix[4] + static_cast<float>(z) * matrix[8] + static_cast<float>(w) * matrix[12],
        static_cast<float>(x) * matrix[1] + static_cast<float>(y) * matrix[5] + static_cast<float>(z) * matrix[9] + static_cast<float>(w) * matrix[13],
        static_cast<float>(x) * matrix[2] + static_cast<float>(y) * matrix[6] + static_cast<float>(z) * matrix[10] + static_cast<float>(w) * matrix[14],
        static_cast<float>(x) * matrix[3] + static_cast<float>(y) * matrix[7] + static_cast<float>(z) * matrix[11] + static_cast<float>(w) * matrix[15]
    );
}

template <typename T>
vec4<T> vec4<T>::multiply(double* matrix) const
{
    /* return Vector4
     (
         static_cast<float>(
         ) * Matrix[0] + static_cast<float>(y) * Matrix[4] + static_cast<float>(z) * Matrix[8] + static_cast<float>(w) * Matrix[12],
         static_cast<float>(x) * Matrix[1] + static_cast<float>(y) * Matrix[5] + static_cast<float>(z) * Matrix[9] + static_cast<float>(w) * Matrix[13],
         static_cast<float>(x) * Matrix[2] + static_cast<float>(y) * Matrix[6] + static_cast<float>(z) * Matrix[10] + static_cast<float>(w) * Matrix[14],
         static_cast<float>(x) * Matrix[3] + static_cast<float>(y) * Matrix[7] + static_cast<float>(z) * Matrix[11] + static_cast<float>(w) * Matrix[15]
     );*/
    return vec4(
        static_cast<float>(x) * matrix[0] + static_cast<float>(y) * matrix[4] + static_cast<float>(z) * matrix[8] + static_cast<float>(w) * matrix[12],
        static_cast<float>(x) * matrix[1] + static_cast<float>(y) * matrix[5] + static_cast<float>(z) * matrix[9] + static_cast<float>(w) * matrix[13],
        static_cast<float>(x) * matrix[2] + static_cast<float>(y) * matrix[6] + static_cast<float>(z) * matrix[10] + static_cast<float>(w) * matrix[14],
        static_cast<float>(x) * matrix[3] + static_cast<float>(y) * matrix[7] + static_cast<float>(z) * matrix[11] + static_cast<float>(w) * matrix[15]
    );
}

template <typename T>
vec4<T> vec4<T>::operator-(const vec4<T>& other) const {
    return vec4(x - other.x, y - other.y, z - other.z, w - other.w);
}

template <typename T>
vec4<T> vec4<T>::operator+(const vec4<T>& other) const {
    return vec4(x + other.x, y + other.y, z + other.z, w + other.w);
}

// Overloaded << operator for Vector4
template <typename T>
std::ostream& operator<<(std::ostream& os, const vec4<T>& vec)
{
    os << "Vector4<" << typeid(T).name() << ">(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
    return os;
}