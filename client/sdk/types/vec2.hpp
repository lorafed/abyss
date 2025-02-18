#pragma once
#define M_PI 3.14159265358979323846
#include <iostream>
#include <cmath>

template <typename T>
class vec2 {
public:
    T x;
    T y;

    vec2();
    vec2(T A, T B);

    T distance(const vec2<T>& other) const;
    vec2 operator-(const vec2<T>& other) const;
    vec2 operator+(const vec2<T>& other) const;
    vec2 rotate(double angle, const vec2<T>& center);
};

template <typename T>
vec2<T>::vec2() : x(0), y(0) {}

template <typename T>
vec2<T>::vec2(T A, T B) : x(A), y(B) {}

template <typename T>
T vec2<T>::distance(const vec2<T>& other) const {
    return sqrt(pow(x - other.x, 2.0) + pow(y - other.y, 2.0));
}

template <typename T>
vec2<T> vec2<T>::operator-(const vec2<T>& other) const {
    return vec2(x - other.x, y - other.y);
}

template <typename T>
vec2<T> vec2<T>::operator+(const vec2<T>& other) const {
    return vec2(x + other.x, y + other.y);
}

template <typename T>
vec2<T> vec2<T>::rotate(double angle, const vec2<T>& center)
{
    double rad = angle * M_PI / 180.0;
    double cos = std::cos(rad);
    double sin = std::sin(rad);

    // Translate the vector to the origin
    T translated_x = x - center.x;
    T translated_y = y - center.y;

    return { translated_x * cos - translated_y * sin + center.x , translated_x * sin + translated_y * cos + center.y };
}

// Overloaded << operator for Vector2
template <typename T>
std::ostream& operator<<(std::ostream& os, const vec2<T>& vec)
{
    os << "Vector2<" << typeid(T).name() << ">(" << vec.x << ", " << vec.y << ")";
    return os;
}
