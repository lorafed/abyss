#include "pch.hpp"
#include "../game_classes.hpp"

bool item_t::is_axe() const
{
	if (!instance_object_) return false;

	return instance_object_.is_instance_of(*axe_class_);
}

bool item_t::is_sword() const
{
	if (!instance_object_) return false;

	return instance_object_.is_instance_of(*sword_class_);
}

bool item_t::is_block() const
{
	if (!instance_object_) return false;

	return instance_object_.is_instance_of(*block_class_);
}

resource_location_t item_t::texture_resource_location() const
{
	if (!instance_object_) resource_location_t();

	defaulted_registry_t item_registry = built_in_registries_t::item_registry();

	resource_location_t resource_location = item_registry.get_item_key(instance_object_);

	std::string namespace_ = resource_location.get_namespace();
	std::string path_ = resource_location.get_path();

	std::string texture_path = is_block() ? "textures/block/" + std::string(path_) + ".png" : "textures/item/" + std::string(path_) + ".png";

	return resource_location_t(namespace_, texture_path);
}

std::string item_t::get_description_id() const
{//getDescriptionId
	if (!instance_object_) {};

	static java_method_t get_description_id_method = get_mapped_method(item_class_, "getDescriptionId", "()Ljava/lang/String;");

	if (!get_description_id_method) {
		return {};
	}

	java_object_t description_id = get_description_id_method.call<jobject>(instance_object_);

	return java_string_t(description_id).to_std_string();
}
