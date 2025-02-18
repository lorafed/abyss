#include "pch.hpp"
#include "game_classes.hpp"

std::vector<entity_t*> level_t::player_list() const
{
	if (!instance_object_) return {};

	JNIEnv* env = java_interop_manager->get_env();

	static java_field_t players_field = get_mapped_field(level_lass_, "players"); //jvm::env->GetFieldID(cache::classes::client_level, mapper::fetch("players").c_str(), "Ljava/util/List;");

	java_object_t players = players_field.get<jobject>(instance_object_);

	static java_method_t to_arr = env->GetMethodID(players, "toArray", "()[Ljava/lang/Object;");

	java_array_t<jobject> arr(to_arr.call<jobject>(players));

	std::vector<entity_t*> entities;
	for (auto& ent : arr.to_vector()) {
		entities.push_back(new entity_t{ ent });
	}

	return entities;
}

item_stack_t inventory_t::get_item(int index) const
{
	if (!instance_object_) return item_stack_t();

	static java_method_t get_item_method = get_mapped_method(inventory_class_, "getItem");

	if (!get_item_method) return item_stack_t();

	java_object_t item = get_item_method.call<jobject>(instance_object_, index);

	return item_stack_t(item);
}

item_stack_t inventory_t::get_armor(int index) const
{
	if (!instance_object_) return item_stack_t();

	static java_method_t get_armor_method = get_mapped_method(inventory_class_, "getArmor");

	if (!get_armor_method) return item_stack_t();

	java_object_t armor = get_armor_method.call<jobject>(instance_object_, index);

	return item_stack_t(armor);
}

std::string resource_location_t::get_namespace() const
{
	if (!instance_object_) return {};

	static java_method_t get_namespace_method = get_mapped_method(resource_location_class_, "getNamespace");
	
	if (!get_namespace_method) return {};

	java_string_t namespace_ = get_namespace_method.call<jobject>(instance_object_);

	return namespace_.to_std_string();
}

std::string resource_location_t::get_path() const
{
	if (!instance_object_) return {};

	static java_method_t get_path_method = get_mapped_method(resource_location_class_, "getPath");

	if (!get_path_method) return {};

	java_string_t path = get_path_method.call<jobject>(instance_object_);

	return path.to_std_string();
}

std::string resource_location_t::to_string() const
{
	if (!instance_object_) return {};

	static java_method_t to_string_method = get_mapped_method(resource_location_class_, "toString");

	if (!to_string_method) return {};

	java_string_t str = to_string_method.call<jobject>(instance_object_);

	return str.to_std_string();
}

defaulted_registry_t built_in_registries_t::item_registry()
{
	built_in_registries_t();

	static java_field_t item_registry_field = get_mapped_field(built_in_registries_class_, "ITEM");

	if (!item_registry_field) return defaulted_registry_t();

	java_object_t item_registry = item_registry_field.get<jobject>();

	return defaulted_registry_t(item_registry);
}

resource_location_t defaulted_registry_t::get_item_key(const item_t& item) const
{
	if (!instance_object_ || !item.instance()) return resource_location_t(); 

	static java_method_t get_key_method = get_mapped_method(defaulted_registry_class_, "getKey");

	if (!get_key_method) return resource_location_t(); 

	java_object_t key = get_key_method.call<jobject>(instance_object_, item.instance().object());

	return resource_location_t(key);
}

std::vector<std::pair<resource_location_t, item_t>> defaulted_registry_t::GetRegistryEntries() {
	std::vector<std::pair<resource_location_t, item_t>> entries;

	JNIEnv* env = java_interop_manager->get_env();

	jmethodID keySetMethod = env->GetMethodID(*defaulted_registry_class_, "keySet", "()Ljava/util/Set;");
	if (!keySetMethod || env->ExceptionCheck()) {
		env->ExceptionClear();
		return entries;
	}

	jobject keySet = env->CallObjectMethod(instance_object_, keySetMethod);
	if (!keySet || env->ExceptionCheck()) {
		env->ExceptionClear();
		return entries;
	}

	jclass setClass = env->FindClass("java/util/Set");
	jmethodID toArrayMethod = env->GetMethodID(setClass, "toArray", "()[Ljava/lang/Object;");
	jobjectArray keyArray = (jobjectArray)env->CallObjectMethod(keySet, toArrayMethod);
	env->DeleteLocalRef(keySet);

	if (!keyArray || env->ExceptionCheck()) {
		env->ExceptionClear();
		return entries;
	}

	jsize arrayLength = env->GetArrayLength(keyArray);


	jmethodID getMethod = env->GetMethodID(*defaulted_registry_class_, "get", "(Lnet/minecraft/resources/ResourceLocation;)Ljava/lang/Object;");
	if (!getMethod || env->ExceptionCheck()) {
		env->ExceptionClear();
		env->DeleteLocalRef(keyArray);
		return entries;
	}

	for (jsize i = 0; i < arrayLength; ++i) {
		jobject resLoc = env->GetObjectArrayElement(keyArray, i);
		if (!resLoc) continue;

		resource_location_t loc = resource_location_t(((java_object_t)resLoc));

		jobject item = env->CallObjectMethod(instance_object_, getMethod, resLoc);
		if (item) {
			entries.emplace_back(loc, item_t(item));
			env->DeleteLocalRef(item); // Release local reference
		}
	}

	env->DeleteLocalRef(keyArray);
	return entries;
}

resource_t resource_manager_t::get_resource(const resource_location_t& resource_location) const
{//resource_location_t
	if (!instance_object_ || !resource_location.instance()) {
		return resource_t();
	}

	static java_method_t get_resource_method = get_mapped_method(reloadable_resource_manager_class_, "getResource");

	if (!get_resource_method) {
		return resource_t();
	}

	java_object_t resource_s = get_resource_method.call<jobject>(instance_object_, (jobject)resource_location.instance());

	static java_method_t get_method = optional_class_->get_method("get", "()Ljava/lang/Object;");
	static java_method_t is_present_method = optional_class_->get_method("isPresent", "()Z");

	if (!is_present_method.call<jboolean>(resource_s)) {
		return resource_t();
	}

	java_object_t resource = get_method.call<jobject>(resource_s);

	return resource_t(resource);
}

std::vector<uint8_t> resource_t::resource_stream() const
{
	if (!instance_object_) return {};

	static java_method_t open_method = get_mapped_method(resource_class_, "open");

	if (!open_method) return {};

	java_object_t input_stream = open_method.call<jobject>(instance_object_);

	static java_method_t read_method = input_stream_class_->get_method("read", "([B)I");

	if (!read_method) return {};

	JNIEnv* env = java_interop_manager->get_env();

	std::vector<uint8_t> buffer;
	jbyteArray byte_array = env->NewByteArray(4096);
	jint bytes_read;

	while ((bytes_read = env->CallIntMethod(input_stream, read_method, byte_array)) > 0)
	{
		jbyte* chunk = env->GetByteArrayElements(byte_array, nullptr);
		buffer.insert(buffer.end(), chunk, chunk + bytes_read);
		env->ReleaseByteArrayElements(byte_array, chunk, JNI_ABORT);
	}

	env->DeleteLocalRef(byte_array);
	env->DeleteLocalRef(input_stream);

	return buffer;
}

int chat_color_t::value() const
{//value
	if (!instance_object_) return {};

	static java_field_t value_field = get_mapped_field(chat_color_class_, "value");

	if (!value_field) return {};

	return value_field.get<jint>(instance_object_);
}

chat_color_t chat_style_t::color() const
{//net/minecraft/network/chat/Style
	if (!instance_object_) return chat_color_t();

	static java_field_t color_field = get_mapped_field(chat_style_class_, "color");

	if (!color_field) return chat_color_t();

	java_object_t key = color_field.get<jobject>(instance_object_);

	return chat_color_t(key);
}

chat_style_t chat_component_t::get_style() const
{
	if (!instance_object_) return chat_style_t();

	static java_method_t get_style_method = get_mapped_method(chat_component_class_, "getStyle");

	if (!get_style_method) return chat_style_t();

	java_object_t style = get_style_method.call<jobject>(instance_object_);

	return chat_style_t(style);
}

std::string chat_component_t::get_string() const
{
	if (!instance_object_) return {};

	static java_method_t get_string_method = get_mapped_method(chat_component_class_, "getString", "()Ljava/lang/String;");

	if (!get_string_method) return {};

	java_object_t style = get_string_method.call<jobject>(instance_object_);

	return java_string_t(style).to_std_string();
}

chat_component_t player_info_t::tab_list_display_name() const
{
	if (!instance_object_) return {};

	static java_field_t tab_list_display_name_field = get_mapped_field(player_info_class_, "tabListDisplayName");

	if (!tab_list_display_name_field) return {};

	java_object_t tab_list_display = tab_list_display_name_field.get<jobject>(instance_object_);

	return chat_component_t(tab_list_display);
}

team_t player_info_t::get_team() const
{
	if (!instance_object_) return team_t();

	static java_method_t get_team_method = get_mapped_method(player_info_class_, "getTeam");

	if (!get_team_method) return team_t();

	java_object_t team = get_team_method.call<jobject>(instance_object_);

	return team_t(team);
}

chat_component_t team_t::player_prefix() const
{
	if (!instance_object_) return chat_component_t();

	static java_field_t player_prefix_field = get_mapped_field(team_class_, "playerPrefix");

	if (!player_prefix_field) return chat_component_t();

	java_object_t player_prefix = player_prefix_field.get<jobject>(instance_object_);

	return chat_component_t(player_prefix);
}

chat_component_t team_t::player_suffix() const
{
	if (!instance_object_) return chat_component_t();

	static java_field_t player_prefix_field = get_mapped_field(team_class_, "playerSuffix");

	if (!player_prefix_field) return chat_component_t();

	java_object_t player_prefix = player_prefix_field.get<jobject>(instance_object_);

	return chat_component_t(player_prefix);
}

chat_formatting_t team_t::team_color() const
{
	if (!instance_object_) return chat_formatting_t();

	static java_field_t color_field = get_mapped_field(team_class_, "color");

	if (!color_field) return chat_formatting_t();

	java_object_t player_prefix = color_field.get<jobject>(instance_object_);

	return chat_formatting_t(player_prefix);
}

int lang_integer_t::value() const
{
	if (!instance_object_) return 0;

	static java_method_t int_value_method = get_mapped_method(lang_integer_class, "intValue");

	if (!int_value_method) return 0;

	return int_value_method.call<jint>(instance_object_);
}

lang_integer_t chat_formatting_t::get_color() const
{
	if (!instance_object_) return lang_integer_t();

	static java_field_t color_field = get_mapped_field(chat_formatting_class_, "color");

	if (!color_field) return lang_integer_t();



	java_object_t color = color_field.get<jobject>(instance_object_);

	return lang_integer_t(color);
}
