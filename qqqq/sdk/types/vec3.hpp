#pragma once
#define M_PI 3.14159265358979323846
#include "vec2.hpp"
#include "vec4.hpp"

template <typename T>
class vec3 {
public:
	T x;
	T y;
	T z;

	vec3();
	vec3(T, T, T);

	template <class U>
	vec3(const vec3<U>&);

	T length() const;
	vec3 normalize() const;

	bool world_to_screen(vec2<float>&, const std::vector<float>&, const std::vector<float>&, const int, const int);
	bool world_to_screen(vec2<float>&, double*, double*, const int, const int);

	bool World2Screen(vec2<float>& ScreenProjection, const std::vector<float>& ModelView, const std::vector<float>& Projection, const int Width, const int Height);

	T distance(const vec3<T>&) const;
	T distance_sq(const vec3<T>&) const;
	vec3 operator-(const vec3<T>&) const;
	vec3 operator+(const vec3<T>&) const;
	vec3 operator*(const vec3<T>&) const;
	vec3 operator*(const T&) const;
	vec3 operator/(const T&) const;

	float get_angles_y_delta(const vec3<T>&, float) const;
	float get_angles_x_delta(const vec3<T>&, float) const;

	float& operator[](int i) {
		return ((float*)this)[i];
	}

	float operator[](int i) const {
		return ((float*)this)[i];
	}

	T length_squared() const {
		return x * x + y * y + z * z;
	}

	// Overloading the equality operator
	bool operator==(const vec3<T>& other) const {
		const float EPSILON = 1e-6f;
		return (std::abs(x - other.x) < EPSILON) &&
			(std::abs(y - other.y) < EPSILON) &&
			(std::abs(z - other.z) < EPSILON);
	}
};

template <typename T>
vec3<T>::vec3() : x(0), y(0), z(0) {}

template <typename T>
vec3<T>::vec3(T A, T B, T C) : x(A), y(B), z(C) {}

template <typename T>
template<typename U>
vec3<T>::vec3(const vec3<U>& other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)), z(static_cast<T>(other.z)) {} // Conversion constructor

template <typename T>
bool vec3<T>::world_to_screen(vec2<float>& screen_projection, const std::vector<float>& model_view, const std::vector<float>& projection, const int width, const int height)
{
	if (projection.size() != 16 || model_view.size() != 16) {
		return false;
	}

	const auto world_pos_vec4 = vec4(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 1.0F).multiply(model_view);
	const auto clip_space_pos = world_pos_vec4.multiply(projection);
	const auto ncd_clip_space_pos = vec4{ static_cast<float>(clip_space_pos.x / clip_space_pos.w), static_cast<float>(clip_space_pos.y / clip_space_pos.w), static_cast<float>(clip_space_pos.z / clip_space_pos.w), static_cast<float>(0.F) };

	if (ncd_clip_space_pos.z < -1.0 || ncd_clip_space_pos.z > 1.0) {
		return false;
	}

	screen_projection.x = width * ((ncd_clip_space_pos.x + 1.0f) / 2.0f);
	screen_projection.y = height * ((1.0f - ncd_clip_space_pos.y) / 2.0f);

	return true;
}

template <typename T>
bool vec3<T>::world_to_screen(vec2<float>& screen_projection, double* model_view, double* projection, const int width, const int height)
{
	const auto world_pos_vec4 = vec4(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 1.0F).multiply(model_view);
	const auto clip_space_pos = world_pos_vec4.multiply(projection);
	const auto ncd_clip_space_pos = vec4{ static_cast<float>(clip_space_pos.x / clip_space_pos.w), static_cast<float>(clip_space_pos.y / clip_space_pos.w), static_cast<float>(clip_space_pos.z / clip_space_pos.w), static_cast<float>(0.F) };

	if (ncd_clip_space_pos.z < -1.0 || ncd_clip_space_pos.z > 1.0) {
		return false;
	}

	screen_projection.x = width * ((ncd_clip_space_pos.x + 1.0f) / 2.0f);
	screen_projection.y = height * ((1.0f - ncd_clip_space_pos.y) / 2.0f);

	return true;
}

template <typename T>
bool vec3<T>::World2Screen(vec2<float>& ScreenProjection, const std::vector<float>& ModelView, const std::vector<float>& Projection, const int Width, const int Height)
{
	if (Projection.size() != 16 || ModelView.size() != 16) {
		return false;
	}

	const auto WorldPositionVec4 = vec4(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 1.0F).Multiply(ModelView).Multiply(Projection);
	const auto ClipSpacePositionVec4 = WorldPositionVec4;
	const auto NCDClipSpacePos = vec4{ static_cast<float>(ClipSpacePositionVec4.x / ClipSpacePositionVec4.w), static_cast<float>(ClipSpacePositionVec4.y / ClipSpacePositionVec4.w), static_cast<float>(ClipSpacePositionVec4.z / ClipSpacePositionVec4.w), static_cast<float>(0.F) };

	if (NCDClipSpacePos.z < -1.0 || NCDClipSpacePos.z > 1.0) {
		return false;
	}

	ScreenProjection.x = Width * ((NCDClipSpacePos.x + 1.0f) / 2.0f);
	ScreenProjection.y = Height * ((1.0f - NCDClipSpacePos.y) / 2.0f);

	return true;
}

template <typename T>
T vec3<T>::distance(const vec3<T>& other) const {
	return sqrtf(static_cast<float>(pow(x - other.x, 2.0) + pow(y - other.y, 2.0) + pow(z - other.z, 2.0)));
}

template <typename T>
T vec3<T>::distance_sq(const vec3<T>& other) const
{
	float dx = x - other.x;
	float dy = y - other.y;
	float dz = z - other.z;
	return dx * dx + dy * dy + dz * dz;
}

template <typename T>
vec3<T> vec3<T>::operator-(const vec3<T>& other) const {
	return vec3(x - other.x, y - other.y, z - other.z);
}

template <typename T>
vec3<T> vec3<T>::operator+(const vec3<T>& other) const {
	return vec3(x + other.x, y + other.y, z + other.z);
}

template<typename T>
vec3<T> vec3<T>::operator*(const vec3<T>& other) const
{
	return vec3(x * other.x, y * other.y, z * other.z);
}

template <typename T>
vec3<T> vec3<T>::operator*(const T& scalar) const {
	return vec3(x * scalar, y * scalar, z * scalar);
}

template <typename T>
vec3<T> vec3<T>::operator/(const T& scalar) const {
	return vec3(x / scalar, y / scalar, z / scalar);
}

template<typename T>
float vec3<T>::get_angles_x_delta(const vec3<T>& target, float angle) const
{
	auto angle_x_difference_to_vec3 = atan2(target.z - this->z, target.x - this->x) * 180. / 3.14159265358979323846 - 90.;

	angle_x_difference_to_vec3 = (double)fmodf((float)angle_x_difference_to_vec3 - angle, 360.f);

	if (angle_x_difference_to_vec3 >= 180.)
		angle_x_difference_to_vec3 -= 360.;

	if (angle_x_difference_to_vec3 < -180.)
		angle_x_difference_to_vec3 += 360.;

	return (float)angle_x_difference_to_vec3;
}

template<typename T>
float vec3<T>::get_angles_y_delta(const vec3<T>& target, float angle) const
{
	return (float)(atan2(this->y - target.y, sqrt(pow(target.x - this->x, 2) + pow(target.z - this->z, 2))) * 180. / 3.14159265358979323846) - angle;
}

// Overloaded << operator for Vector3
template <typename T>
std::ostream& operator<<(std::ostream& os, const vec3<T>& vec)
{
	os << "Vector3<" << typeid(T).name() << ">(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	return os;
}

template <typename T>
T vec3<T>::length() const {
	return static_cast<T>(sqrt(x * x + y * y + z * z));
}

template <typename T>
vec3<T> vec3<T>::normalize() const 
{
	T len = length();
	if (len == 0) return *this;  // Avoid division by zero, return the zero vector
	return vec3(x / len, y / len, z / len);
}