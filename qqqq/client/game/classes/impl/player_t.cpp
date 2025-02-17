#include "pch.hpp"
#include "../game_classes.hpp"

aabb_t entity_t::bb() const
{
    if (!instance_object_) return aabb_t();

    static java_field_t bb_field = get_mapped_field(entity_class_, "bb");

    if (!bb_field) return aabb_t();

    java_object_t bb = bb_field.get<jobject>(instance_object_);
    
    return aabb_t(bb);
}

float entity_t::get_health() const
{
	if (!instance_object_) return 0.F;

	static java_method_t get_health_method = get_mapped_method(living_entity_class_, "getHealth", "()F");

	if (!get_health_method) return 0.F;

	return get_health_method.call<jfloat>(instance_object_);
}

item_stack_t entity_t::get_main_hand_item() const
{
    if (!instance_object_) return item_stack_t();

    static java_method_t get_main_hand_item_method = get_mapped_method(living_entity_class_, "getMainHandItem");

    if (!get_main_hand_item_method) return item_stack_t();

    java_object_t main_hand_item = get_main_hand_item_method.call<jobject>(instance_object_);

    return item_stack_t(main_hand_item);
}

vec3<double> entity_t::position() const
{
	if (!instance_object_) return {};

	static java_field_t position_field = get_mapped_field(entity_class_, "position");

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

vec3<double> entity_t::last_position() const
{
	if (!instance_object_) return {};

	static java_field_t x_field = get_mapped_field(entity_class_, "xo");
	static java_field_t y_field = get_mapped_field(entity_class_, "yo");
	static java_field_t z_field = get_mapped_field(entity_class_, "zo");

	if (!x_field || !y_field || !z_field) return {};

	return
	{
		(float)x_field.get<jdouble>(instance_object_),
		(float)y_field.get<jdouble>(instance_object_),
		(float)z_field.get<jdouble>(instance_object_),
	};
}

inventory_t entity_t::inventory() const
{
	if (!instance_object_) return inventory_t();

	static java_method_t get_inventory = get_mapped_method(player_class_, "getInventory");

	if (!get_inventory) return inventory_t();

	java_object_t inventory = get_inventory.call<jobject>(instance_object_);

	return inventory_t(inventory);
}

chat_component_t entity_t::get_display_name() const
{
	if (!instance_object_) return chat_component_t();

	static java_method_t get_display_name_method = get_mapped_method(player_class_, "getDisplayName");

	if (!get_display_name_method) return chat_component_t();

	java_object_t inventory = get_display_name_method.call<jobject>(instance_object_);

	return chat_component_t(inventory);
}

std::string entity_t::get_scoreboard_name() const
{
	if (!instance_object_) return {};

	static java_method_t get_scoreboard_name_method = get_mapped_method(entity_class_, "getScoreboardName", "()Ljava/lang/String;");

	if (!get_scoreboard_name_method) return {};

	java_object_t score_board_name = get_scoreboard_name_method.call<jobject>(instance_object_);

	return java_string_t(score_board_name).to_std_string();
}

player_info_t entity_t::get_player_info() const
{
	if (!instance_object_) return player_info_t();

	static java_method_t get_player_info_method = get_mapped_method(abstract_client_player_, "getPlayerInfo");

	if (!get_player_info_method) return player_info_t();

	java_object_t player_info = get_player_info_method.call<jobject>(instance_object_);

	return player_info_t(player_info);
}

void entity_t::set_shared_flag(int flag, bool state) const
{
	if (!instance_object_) return;

	static java_method_t set_shared_flag_method = get_mapped_method(entity_class_, "setSharedFlag", "(IZ)V");

	if (!set_shared_flag_method) return;

	set_shared_flag_method.call<void>(instance_object_, flag, state);
}
