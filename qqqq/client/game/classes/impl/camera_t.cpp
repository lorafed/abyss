#include "pch.hpp"
#include "game_classes.hpp"

vec3<float> camera_t::get_position() const
{
	if (!instance_object_) return {};

	static java_field_t position_field = get_mapped_field(camera_class_, "position");

	java_object_t position = position_field.get<jobject>(instance_object_);

	static java_field_t vec3_x_field = get_mapped_field(vec3_class_, "x");
	static java_field_t vec3_y_field = get_mapped_field(vec3_class_, "y");
	static java_field_t vec3_z_field = get_mapped_field(vec3_class_, "z");

	if (!vec3_x_field || !vec3_y_field || !vec3_z_field) return {};

	return
	{
		(float)vec3_x_field.get<jdouble>(position),
		(float)vec3_y_field.get<jdouble>(position),
		(float)vec3_z_field.get<jdouble>(position),
	};
}

vec2<float> camera_t::get_rotation() const
{
	if (!instance_object_) return {};

	static java_field_t rot_x_field = get_mapped_field(camera_class_, "xRot");
	static java_field_t rot_y_field = get_mapped_field(camera_class_, "yRot");

	if (!rot_y_field || !rot_x_field) return {};

	return
	{
		rot_x_field.get<jfloat>(instance_object_),
		rot_y_field.get<jfloat>(instance_object_)
	};
}

float camera_t::get_partial_ticks() const
{
	if (!instance_object_) return 0.F;

	static java_field_t partial_tick_time_field = get_mapped_field(camera_class_, "partialTickTime");

	if (!partial_tick_time_field) return 0.F;

	return partial_tick_time_field.get<jfloat>(instance_object_);
}

float camera_t::get_eye_height() const
{
	if (!instance_object_) return 0.F;

	static java_field_t eye_height_field = get_mapped_field(camera_class_, "eyeHeight");

	if (!eye_height_field) return 0.F;

	return eye_height_field.get<jfloat>(instance_object_);
}
