#pragma once
#include <algorithm>

#include "vec3.hpp"

#undef min
#undef max

class ray_trace_t
{
public:
    vec3<float> m_origin;
    vec3<float> m_dir;
    vec3<float> m_dir_inv;

    enum class face
    {
        East,
        West,
        North,
        South,
        Up,
        Down,
        None
    };

    explicit ray_trace_t(const vec3<float>& origin, const vec3<float>& dir)
    {
        m_origin = origin;
        m_dir = dir;

        // Ensure that direction is not zero to avoid division by zero
        const float EPSILON = 1e-6f;
        m_dir_inv.x = (std::abs(m_dir.x) > EPSILON) ? 1.f / m_dir.x : 0.f;
        m_dir_inv.y = (std::abs(m_dir.y) > EPSILON) ? 1.f / m_dir.y : 0.f;
        m_dir_inv.z = (std::abs(m_dir.z) > EPSILON) ? 1.f / m_dir.z : 0.f;
    }

    bool trace(const vec3<float>& mins, const vec3<float>& maxs, float& distance, vec3<float>& end) const
    {
        const float EPSILON = 1e-6f;
        if (std::abs(m_dir.x) < EPSILON && (m_origin.x < mins.x || m_origin.x > maxs.x)) return false;
        if (std::abs(m_dir.y) < EPSILON && (m_origin.y < mins.y || m_origin.y > maxs.y)) return false;
        if (std::abs(m_dir.z) < EPSILON && (m_origin.z < mins.z || m_origin.z > maxs.z)) return false;

        const float t1 = (mins.x - m_origin.x) * m_dir_inv.x;
        const float t2 = (maxs.x - m_origin.x) * m_dir_inv.x;
        const float t3 = (mins.y - m_origin.y) * m_dir_inv.y;
        const float t4 = (maxs.y - m_origin.y) * m_dir_inv.y;
        const float t5 = (mins.z - m_origin.z) * m_dir_inv.z;
        const float t6 = (maxs.z - m_origin.z) * m_dir_inv.z;

        const float tmin_x = std::min(t1, t2);
        const float tmax_x = std::max(t1, t2);
        const float tmin_y = std::min(t3, t4);
        const float tmax_y = std::max(t3, t4);
        const float tmin_z = std::min(t5, t6);
        const float tmax_z = std::max(t5, t6);

        const float tmin = std::max(tmin_x, std::max(tmin_y, tmin_z));
        const float tmax = std::min(tmax_x, std::min(tmax_y, tmax_z));

        if (tmax < 0.f) return false;
        if (tmin > tmax) return false;

        distance = tmin;
        end = m_dir * distance + m_origin;
        return true;
    }

    bool trace(const vec3<float>& mins, const vec3<float>& maxs, float& distance, vec3<float>& end, face& hit_face) const
    {
        const float EPSILON = 1e-6f;
        if (std::abs(m_dir.x) < EPSILON && (m_origin.x < mins.x || m_origin.x > maxs.x)) return false;
        if (std::abs(m_dir.y) < EPSILON && (m_origin.y < mins.y || m_origin.y > maxs.y)) return false;
        if (std::abs(m_dir.z) < EPSILON && (m_origin.z < mins.z || m_origin.z > maxs.z)) return false;

        const float t1 = (mins.x - m_origin.x) * m_dir_inv.x;
        const float t2 = (maxs.x - m_origin.x) * m_dir_inv.x;
        const float t3 = (mins.y - m_origin.y) * m_dir_inv.y;
        const float t4 = (maxs.y - m_origin.y) * m_dir_inv.y;
        const float t5 = (mins.z - m_origin.z) * m_dir_inv.z;
        const float t6 = (maxs.z - m_origin.z) * m_dir_inv.z;

        const float tmin_x = std::min(t1, t2);
        const float tmax_x = std::max(t1, t2);
        const float tmin_y = std::min(t3, t4);
        const float tmax_y = std::max(t3, t4);
        const float tmin_z = std::min(t5, t6);
        const float tmax_z = std::max(t5, t6);

        const float tmin = std::max(tmin_x, std::max(tmin_y, tmin_z));
        const float tmax = std::min(tmax_x, std::min(tmax_y, tmax_z));

        if (tmax < 0.f || tmin > tmax) return false;

        distance = tmin;
        end = m_dir * distance + m_origin;

        // Determine which face was hit based on which `tmin` value contributed to the hit
        if (tmin == t1) hit_face = face::West;
        else if (tmin == t2) hit_face = face::East;
        else if (tmin == t3) hit_face = face::Down;
        else if (tmin == t4) hit_face = face::Up;
        else if (tmin == t5) hit_face = face::North;
        else if (tmin == t6) hit_face = face::South;
        else hit_face = face::None;

        return true;
    }
};
