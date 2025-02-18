#include "pch.hpp"
#include "../game_classes.hpp"

item_t item_stack_t::get_item() const
{
	if (!instance_object_) return item_t();

	static java_method_t get_item_method = get_mapped_method(item_stack_class_, "getItem");

	if (!get_item_method) return item_t();

	java_object_t item = get_item_method.call<jobject>(instance_object_);

	return item_t(item);
}
