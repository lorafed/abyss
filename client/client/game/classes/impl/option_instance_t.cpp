#include "pch.hpp"
#include "game_classes.hpp"

float option_instance_t::get_value() const
{
	if (!instance_object_) return 0.0;

	static java_field_t value_field = get_mapped_field(option_instance_class_, "value");

	if (!value_field) return 0.0;

	java_object_t value_obj = value_field.get<jobject>(instance_object_);

	if (!value_obj.is_instance_of_t(*numbers_class_)) return 0.0;

	static java_method_t float_value_method = numbers_class_->get_method("floatValue");

	if (!float_value_method) return 0.0;

	return float_value_method.call<jfloat>(value_obj);
}

void option_instance_t::set_value(double val) const
{
	if (!instance_object_) return;

	static java_field_t value_field = get_mapped_field(option_instance_class_, "value");

	if (!value_field) {
		return;
	}

	JNIEnv* env = java_interop_manager->get_env();
	jmethodID initializer = env->GetMethodID(*double_class_, "<init>", "(D)V");

	java_object_t new_ = env->NewObject(*double_class_, initializer, val);

	if (new_) {
		value_field.set<jobject>(instance_object_, (jobject)new_);
	}
}
