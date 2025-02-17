#include "pch.hpp"
#include "game_classes.hpp"

bool key_mapping_t::is_down() const
{
	if (!instance_object_) return false;

	static java_method_t is_down_method = get_mapped_method(key_mapping_class_, "isDown");

	if (!is_down_method) return false;

	return is_down_method.call<jboolean>(instance_object_);
}

void key_mapping_t::set_down(bool state) const
{
	if (!instance_object_) return;

	//badlion likes to be weird
	if (ctx.is_badlion())
	{
		JNIEnv* env = java_interop_manager->get_env();
		static java_method_t set_down_method = env->GetMethodID(*key_mapping_class_, mapper->get_method_mapping("net.minecraft.client.KeyMapping", "setDown").data(), "(Z)V");

		if (!set_down_method) return;

		set_down_method.call<void>(instance_object_, state);
		return;
	}

	JNIEnv* env = java_interop_manager->get_env();
	static java_method_t set_down_method = env->GetMethodID(*key_mapping_class_, "setDown", "(Z)V");
	set_down_method.call<void>(instance_object_, state);
}