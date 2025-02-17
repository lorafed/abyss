#include "pch.hpp"
#include "../game_classes.hpp"

minecraft_t::minecraft_t()
{
    INITIALIZE_CLASS(minecraft_class_, "net.minecraft.client.Minecraft");

    static java_method_t get_instance_method = get_mapped_method(minecraft_class_, "getInstance");

    if (get_instance_method == nullptr) {
        JNIEnv* env = java_interop_manager->get_env();
        jclass clazz = java_interop_manager->find_class("net.minecraft.client.Minecraft");

        java_class_t clzz(clazz);

        clzz.debug_print();
        //jmethodID method = env->GetMethodID(clazz, "getInstance", "()Lnet/minecraft/client/Minecraft;");

      //  std::cout << method << std::endl;
        return;
    }

    instance_object_ = get_instance_method.call<jobject>();
}

entity_t minecraft_t::player() const
{
    if (!instance_object_) return entity_t();

    static java_field_t player_field = get_mapped_field(minecraft_class_, "player");

    if (!player_field) return entity_t();

    java_object_t hit_result = player_field.get<jobject>(instance_object_);

    return entity_t(hit_result);
}

entity_t minecraft_t::camera_entity() const
{
    if (!instance_object_) return entity_t();

    static java_field_t camera_entity_field = get_mapped_field(minecraft_class_, "cameraEntity");

    if (!camera_entity_field) return entity_t();

    java_object_t hit_result = camera_entity_field.get<jobject>(instance_object_);

    return entity_t(hit_result);
}

hit_result_t minecraft_t::hit_result() const
{
    if (!instance_object_) return hit_result_t();

    static java_field_t hit_result_field = get_mapped_field(minecraft_class_, "hitResult");

    if (!hit_result_field) return hit_result_t();

    java_object_t hit_result = hit_result_field.get<jobject>(instance_object_);

    return hit_result_t(hit_result);
}

screen_t minecraft_t::screen() const
{
    if (!instance_object_) return screen_t();

    static java_field_t screen_field = get_mapped_field(minecraft_class_, "screen");

    if (!screen_field) return screen_t();

    java_object_t screen = screen_field.get<jobject>(instance_object_);

    return screen_t(screen);
}

options_t minecraft_t::options() const
{
    if (!instance_object_) return options_t();

    static java_field_t options_field = get_mapped_field(minecraft_class_, "options");

    if (!options_field) return options_t();

    java_object_t screen = options_field.get<jobject>(instance_object_);

    return options_t(screen);
}

level_t minecraft_t::level() const
{
    if (!instance_object_) return level_t();

    static java_field_t level_field = get_mapped_field(minecraft_class_, "level");

    if (!level_field) return level_t();

    java_object_t screen = level_field.get<jobject>(instance_object_);

    return level_t(screen);
}

entity_render_dispatcher_t minecraft_t::entity_render_dispatcher() const
{
    if (!instance_object_) return entity_render_dispatcher_t();

    static java_field_t options_field = get_mapped_field(minecraft_class_, "entityRenderDispatcher");

    if (!options_field) return entity_render_dispatcher_t();

    java_object_t screen = options_field.get<jobject>(instance_object_);

    return entity_render_dispatcher_t(screen);
}

resource_manager_t minecraft_t::resource_manager() const
{
    if (!instance_object_) return resource_manager_t();

    static java_method_t get_resource_manager_method = get_mapped_method(minecraft_class_, "getResourceManager");

    if (!get_resource_manager_method) return resource_manager();

    java_object_t screen = get_resource_manager_method.call<jobject>(instance_object_);

    return resource_manager_t(screen);
}
