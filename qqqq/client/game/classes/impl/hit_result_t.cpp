#include "pch.hpp"
#include "../game_classes.hpp"

bool hit_result_t::is_block() const
{
    if (!instance_object_) return false;

    static java_method_t get_type_method = get_mapped_method(hit_result_class_, "getType");
    static java_field_t block_field = get_mapped_field(hit_result_type_class_, "BLOCK");

    if (!get_type_method || !block_field) return false;

    java_object_t curr_type = get_type_method.call<jobject>(instance_object_);
    java_object_t block_type = block_field.get<jobject>();

    if (!curr_type || !block_type) return false;

    return curr_type.is_same_object_t(block_type);
}

bool hit_result_t::is_entity() const
{
    if (!instance_object_) return false;

    static java_method_t get_type_method = get_mapped_method(hit_result_class_, "getType");
    static java_field_t entity_field = get_mapped_field(hit_result_type_class_, "ENTITY");

    if (!get_type_method || !entity_field) return false;

    java_object_t curr_type = get_type_method.call<jobject>(instance_object_);
    java_object_t entity_type = entity_field.get<jobject>();

    if (!curr_type || !entity_type) return false;

    return curr_type.is_same_object_t(entity_type);
}
