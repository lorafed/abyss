#include "pch.hpp"
#include "../game_classes.hpp"

bool screen_t::is_inventory() const
{
	if (!instance_object_) return false;

	return instance_object_.is_instance_of_t(*inventory_screen_class_);
}
