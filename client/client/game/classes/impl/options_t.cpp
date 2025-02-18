#include "pch.hpp"
#include "game_classes.hpp"

option_instance_t options_t::gamma() const
{
	if (!instance_object_) return option_instance_t();

	static java_field_t gamma_field = get_mapped_field(option_class_, "gamma");

	if (!gamma_field) {
	
		return option_instance_t();
	}

	java_object_t gamma = gamma_field.get<jobject>(instance_object_);

	return option_instance_t(gamma);
}

option_instance_t options_t::fov() const
{
	if (!instance_object_) return option_instance_t();

	static java_field_t fov_field = get_mapped_field(option_class_, "fov");

	if (!fov_field) {

		return option_instance_t();
	}

	java_object_t fov = fov_field.get<jobject>(instance_object_);

	return option_instance_t(fov);
}

key_mapping_t options_t::key_sprint() const
{
	if (!instance_object_) return key_mapping_t();

	static java_field_t key_sprint_field = get_mapped_field(option_class_, "keySprint");

	if (!key_sprint_field) {

		return key_mapping_t();
	}

	java_object_t key_sprint = key_sprint_field.get<jobject>(instance_object_);

	return key_mapping_t(key_sprint);
}
