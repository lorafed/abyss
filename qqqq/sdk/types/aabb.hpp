#pragma once
#define M_PI 3.14159265358979323846
#include <iostream>

#include "vec2.hpp"
#include "vec3.hpp"
#include "vec4.hpp"

template<typename T>
class aabb {
public:
    aabb() {};
    aabb(T MinX, T MinY, T MinZ, T MaxX, T MaxY, T MaxZ) : min_x(MinX), min_y(MinY), min_z(MinZ), max_x(MaxX), max_y(MaxY), max_z(MaxZ) 
    {
        this->min = vec3<T>{ MinX, MinY, MinZ };
        this->max = vec3<T>{ MaxX, MaxY, MaxZ };
    }

    T get_min_x() const { return min_x; }
    T get_min_y() const { return min_y; }
    T get_min_z() const { return min_z; }
    T get_max_x() const { return max_x; }
    T get_max_y() const { return max_y; }
    T get_max_z() const { return max_z; }

    void set_min_x(T MinX) { this->min_x = MinX; }
    void set_min_y(T MinY) { this->min_y = MinY; }
    void set_min_z(T MinZ) { this->min_z = MinZ; }
    void set_max_x(T MaxX) { this->max_x = MaxX; }
    void set_max_y(T MaxY) { this->max_y = MaxY; }
    void set_max_z(T MaxZ) { this->max_z = MaxZ; }

    aabb<T> get_bounding_box_3d(const vec3<float>& pos, const vec3<float>& last_pos, const vec3<float>& render_pos, const float render_partial_ticks)
    {
        return aabb<T>
        {
            this->get_min_x() - render_pos.x + -pos.x + last_pos.x + (pos.x - last_pos.x) * render_partial_ticks,
            this->get_min_y() - render_pos.y + -pos.y + last_pos.y + (pos.y - last_pos.y) * render_partial_ticks,
            this->get_min_z() - render_pos.z + -pos.z + last_pos.z + (pos.z - last_pos.z) * render_partial_ticks,
            this->get_max_x() - render_pos.x + -pos.x + last_pos.x + (pos.x - last_pos.x) * render_partial_ticks,
            this->get_max_y() - render_pos.y + -pos.y + last_pos.y + (pos.y - last_pos.y) * render_partial_ticks,
            this->get_max_z() - render_pos.z + -pos.z + last_pos.z + (pos.z - last_pos.z) * render_partial_ticks

        };
    }

    aabb<T> get_bounding_box_3d(const vec3<double>& render_pos)
    {
        return aabb<T>
        {
            this->get_min_x() - render_pos.x,
            this->get_min_y() - render_pos.y,
            this->get_min_z() - render_pos.z,
            this->get_max_x() - render_pos.x,
            this->get_max_y() - render_pos.y,
            this->get_max_z() - render_pos.z

        };
    }

    vec4<float> get_screen_position(double* projection, double* model_view, const int height, const int width)
    {
        const std::vector<vec3<float>> vert_2d
        {
            {static_cast<float>(min_x), static_cast<float>(min_y), static_cast<float>(min_z)},
            {static_cast<float>(min_x), static_cast<float>(max_y), static_cast<float>(min_z)},
            {static_cast<float>(max_x), static_cast<float>(max_y), static_cast<float>(min_z)},
            {static_cast<float>(max_x), static_cast<float>(min_y), static_cast<float>(min_z)},
            {static_cast<float>(min_x), static_cast<float>(min_y), static_cast<float>(max_z)},
            {static_cast<float>(min_x), static_cast<float>(max_y), static_cast<float>(max_z)},
            {static_cast<float>(max_x), static_cast<float>(max_y), static_cast<float>(max_z)},
            {static_cast<float>(max_x), static_cast<float>(min_y), static_cast<float>(max_z)},


            /*Vector3<float>{MinX, MinY, MinZ},
            Vector3<float>{MinX, MaxY, MinZ},
            Vector3<float>{MaxX, MaxY, MinZ},
            Vector3<float>{MaxX, MinY, MinZ},
            Vector3<float>{MinX, MinY, MaxZ},
            Vector3<float>{MinX, MaxY, MaxZ},
            Vector3<float>{MaxX, MaxY, MaxZ},
            Vector3<float>{MaxX, MinY, MaxZ},*/
        };

        vec4 screen_vert = { FLT_MAX, FLT_MAX, -1.F, -1.F };

        for (int vert_idx = 0; vert_idx < 8; vert_idx++)
        {
            vec2<float> screen_pos;
            auto current_vert = vert_2d[vert_idx];

            if (!current_vert.world_to_screen(screen_pos, model_view, projection, width, height)) {
                continue;
            }

            screen_vert.x = fminf(screen_pos.x, screen_vert.x);
            screen_vert.y = fminf(screen_pos.y, screen_vert.y);
            screen_vert.z = fmaxf(screen_pos.x, screen_vert.z);
            screen_vert.w = fmaxf(screen_pos.y, screen_vert.w);
        }

        return screen_vert;
    }

    vec4<float> get_screen_position(const std::vector<float>& projection, const std::vector<float>& model_view, const int height, const int width)
    {
        const std::vector<vec3<float>> vert_2d
        {
            {static_cast<float>(min_x), static_cast<float>(min_y), static_cast<float>(min_z)},
            {static_cast<float>(min_x), static_cast<float>(max_y), static_cast<float>(min_z)},
            {static_cast<float>(max_x), static_cast<float>(max_y), static_cast<float>(min_z)},
            {static_cast<float>(max_x), static_cast<float>(min_y), static_cast<float>(min_z)},
            {static_cast<float>(min_x), static_cast<float>(min_y), static_cast<float>(max_z)},
            {static_cast<float>(min_x), static_cast<float>(max_y), static_cast<float>(max_z)},
            {static_cast<float>(max_x), static_cast<float>(max_y), static_cast<float>(max_z)},
            {static_cast<float>(max_x), static_cast<float>(min_y), static_cast<float>(max_z)},


            /*Vector3<float>{MinX, MinY, MinZ},
            Vector3<float>{MinX, MaxY, MinZ},
            Vector3<float>{MaxX, MaxY, MinZ},
            Vector3<float>{MaxX, MinY, MinZ},
            Vector3<float>{MinX, MinY, MaxZ},
            Vector3<float>{MinX, MaxY, MaxZ},
            Vector3<float>{MaxX, MaxY, MaxZ},
            Vector3<float>{MaxX, MinY, MaxZ},*/
        };

        vec4 screen_vert = { FLT_MAX, FLT_MAX, -1.F, -1.F };

        for (int vert_idx = 0; vert_idx < 8; vert_idx++)
        {
            vec2<float> screen_pos;
            auto current_vert = vert_2d[vert_idx];

            if (!current_vert.world_to_screen(screen_pos, model_view, projection, width, height)) {
                continue;
            }

            screen_vert.x = fminf(screen_pos.x, screen_vert.x);
            screen_vert.y = fminf(screen_pos.y, screen_vert.y);
            screen_vert.z = fmaxf(screen_pos.x, screen_vert.z);
            screen_vert.w = fmaxf(screen_pos.y, screen_vert.w);
        }

        return screen_vert;
    }

    bool contains(T x, T y, T Z) const {
        return x >= min_x && x <= max_x && y >= min_y && y <= max_y && Z >= min_z && Z <= max_z;
    }

    aabb<T> operator+(const aabb<T>& other) const {
        return aabb<T>(min_x + other.min_x, min_y + other.min_y, min_z + other.min_z, max_x + other.max_x, max_y + other.max_y, max_z + other.max_z);
    }

    aabb<T> operator-(const aabb<T>& other) const {
        return aabb<T>(min_x - other.min_x, min_y - other.min_y, min_z - other.min_z, max_x - other.max_x, max_y - other.max_y, max_z - other.max_z);
    }

    aabb<T> operator*(const float other) const {
        return aabb<T>(min_x * other, min_y * other, min_z * other, max_x * other, max_y * other, max_z * other);
    }

    aabb<T> expand(const float other) const {
        return aabb<T>(min_x - other, min_y - other, min_z - other, max_x + other, max_y + other, max_z + other);
    }

    void set_position(vec3<double> pos)
    {
        double width = max_x - min_x;
        double height = max_y - min_y;
        double depth = max_z - min_z;

        min_x = pos.x - width / 2;
        min_y = pos.y;
        min_z = pos.z - depth / 2;

        max_x = pos.x + width / 2;
        max_y = pos.y + height;
        max_z = pos.z + depth / 2;
    }

    T min_x, min_y, min_z;
    T max_x, max_y, max_z;

    vec3<T> min;
    vec3<T> max;
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const aabb<T>& aabb)
{
    os << "AxisAlignedBB: "
        << "[" << aabb.get_min_x() << ", " << aabb.get_min_y() << ", " << aabb.get_min_z() << "] - "
        << "[" << aabb.get_max_x() << ", " << aabb.get_max_y() << ", " << aabb.get_max_z() << "]";

    return os;
}