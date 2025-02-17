#include "pch.hpp"
#include "game_classes.hpp"

camera_t entity_render_dispatcher_t::camera() const
{
	if (!instance_object_) return camera_t();

	static java_field_t camera_field = get_mapped_field(entity_render_dispatcher_class_, "camera");

	if (!camera_field) return camera_t();

	java_object_t camera = camera_field.get<jobject>(instance_object_);

	return camera_t(camera);
}