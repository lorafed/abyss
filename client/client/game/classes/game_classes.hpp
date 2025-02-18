#pragma once
#include "../mapped_class.hpp"

#include "types/vec2.hpp"
#include "types/vec3.hpp"
#include "types/vec4.hpp"
#include "types/aabb.hpp"

class item_t;

class chat_color_t : public mapped_class_t {
public:  
    chat_color_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(chat_color_class_, "net.minecraft.network.chat.TextColor");
        instance_object_ = instance;
    }

    chat_color_t() {};

    int value() const;

private:
    inline static std::shared_ptr<::java_class_t> chat_color_class_;
};


class chat_style_t : public mapped_class_t {
public:
    chat_style_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(chat_style_class_, "net.minecraft.network.chat.Style");
        instance_object_ = instance;
    }

    chat_style_t() {};

    chat_color_t color() const;

private:
    inline static std::shared_ptr<::java_class_t> chat_style_class_;
};


class chat_component_t : public mapped_class_t {
public: 
    chat_component_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(chat_component_class_, "net.minecraft.network.chat.Component");
        instance_object_ = instance;
    }

    chat_component_t() {};

    chat_style_t get_style() const;
    std::string get_string() const;

private:
    inline static std::shared_ptr<::java_class_t> chat_component_class_;
};

class resource_location_t : public mapped_class_t {
public:
    resource_location_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(resource_location_class_, "net.minecraft.resources.ResourceLocation");
        instance_object_ = instance;
    }

    resource_location_t(const std::string& tex_namespace, const std::string& tex_path)
    {
        INITIALIZE_CLASS(resource_location_class_, "net.minecraft.resources.ResourceLocation");

        static java_method_t initializer = resource_location_class_->get_method("<init>", "(Ljava/lang/String;Ljava/lang/String;)V");

        if (!initializer) {
            return;
        }

        JNIEnv* env = java_interop_manager->get_env();

        java_string_t namespace_ = java_string_t::from_std_string(tex_namespace);
        java_string_t path_ = java_string_t::from_std_string(tex_path);

        instance_object_ = env->NewObject(*resource_location_class_, (jmethodID)initializer, namespace_.object(), path_.object());
    }

    std::string get_namespace() const;
    std::string get_path() const;
    std::string to_string() const;

    resource_location_t() {};
private:
    inline static std::shared_ptr<::java_class_t> resource_location_class_;
};

class defaulted_registry_t : public mapped_class_t {
public:
    defaulted_registry_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(defaulted_registry_class_, "net.minecraft.core.DefaultedRegistry");
        instance_object_ = instance;
    }

    defaulted_registry_t() {};

    resource_location_t get_item_key(const item_t& item) const;

    std::vector<std::pair<resource_location_t, item_t>> GetRegistryEntries();

private:
    inline static std::shared_ptr<::java_class_t> defaulted_registry_class_;
};

class built_in_registries_t : public mapped_class_t {
public:
    built_in_registries_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(built_in_registries_class_, "net.minecraft.core.registries.BuiltInRegistries");
        instance_object_ = instance;
    }

    built_in_registries_t() : mapped_class_t()
    {
        INITIALIZE_CLASS(built_in_registries_class_, "net.minecraft.core.registries.BuiltInRegistries");
    }

    static defaulted_registry_t item_registry();

private:
    inline static std::shared_ptr<::java_class_t> built_in_registries_class_;
};

class resource_t : public mapped_class_t {
public:
    resource_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(resource_class_, "net.minecraft.server.packs.resources.Resource");
        FIND_CLASS(input_stream_class_, "java.io.InputStream");
        instance_object_ = instance;
    }

    std::vector<uint8_t> resource_stream() const;

    resource_t() {};
private:
    inline static std::shared_ptr<::java_class_t> resource_class_;
    inline static std::shared_ptr<::java_class_t> input_stream_class_;
};

class resource_manager_t : public mapped_class_t {
public:
    resource_manager_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(reloadable_resource_manager_class_, "net.minecraft.server.packs.resources.ReloadableResourceManager");
        INITIALIZE_CLASS(resource_manager_class_, "net.minecraft.server.packs.resources.ResourceManager");
        FIND_CLASS(input_stream_class_, "java.io.InputStream");
        FIND_CLASS(optional_class_, "java.util.Optional");
        instance_object_ = instance;
    }

    resource_manager_t() {};

    resource_t get_resource(const resource_location_t& resource_location) const;


    

private:
    inline static std::shared_ptr<::java_class_t> reloadable_resource_manager_class_;
    inline static std::shared_ptr<::java_class_t> resource_manager_class_;
    inline static std::shared_ptr<::java_class_t> input_stream_class_;
    inline static std::shared_ptr<::java_class_t> optional_class_;
};

class hit_result_t : public mapped_class_t {
public:
    hit_result_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(hit_result_class_, "net.minecraft.world.phys.HitResult");
        INITIALIZE_CLASS(hit_result_type_class_, "net.minecraft.world.phys.HitResult$Type");
        instance_object_ = instance;
    }

    hit_result_t() {};

    bool is_block() const;
    bool is_entity() const;
private:
    inline static std::shared_ptr<::java_class_t> hit_result_class_;
    inline static std::shared_ptr<::java_class_t> hit_result_type_class_;
};

class screen_t : public mapped_class_t {
public:
    screen_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(screen_class_, "net.minecraft.client.gui.screens.Screen");
        INITIALIZE_CLASS(inventory_screen_class_, "net.minecraft.client.gui.screens.inventory.InventoryScreen");
        instance_object_ = instance;
    }

    screen_t() {};

    bool is_inventory() const;
private:
    inline static std::shared_ptr<::java_class_t> screen_class_;
    inline static std::shared_ptr<::java_class_t> inventory_screen_class_;
};

class item_t : public mapped_class_t {
public:
    item_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(item_class_, "net.minecraft.world.item.Item");

        INITIALIZE_CLASS(axe_class_, "net.minecraft.world.item.AxeItem");
        INITIALIZE_CLASS(sword_class_, "net.minecraft.world.item.SwordItem");
        INITIALIZE_CLASS(block_class_, "net.minecraft.world.item.BlockItem");

        instance_object_ = instance;
    }

    bool is_axe() const;
    bool is_sword() const;
    bool is_block() const;

    resource_location_t texture_resource_location() const;
    std::string get_description_id() const;

    item_t() {};
private:
    inline static std::shared_ptr<::java_class_t> item_class_;

    inline static std::shared_ptr<::java_class_t> axe_class_;
    inline static std::shared_ptr<::java_class_t> sword_class_;
    inline static std::shared_ptr<::java_class_t> block_class_;
};

class item_stack_t : public mapped_class_t {
public:
    item_stack_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(item_stack_class_, "net.minecraft.world.item.ItemStack");
        instance_object_ = instance;
    }

    item_stack_t() {};
   
    item_t get_item() const;
private:
    inline static std::shared_ptr<::java_class_t> item_stack_class_;
};

class aabb_t : public mapped_class_t
{
public:
    aabb_t(java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(aabb_class_, "net.minecraft.world.phys.AABB");
        instance_object_ = instance;
    }

    aabb_t() {};

    aabb<double> bounds() const;
    void set_bounds(const aabb<double>& bounds) const;

private:
    inline static std::shared_ptr<::java_class_t> aabb_class_;
};

class inventory_t : public mapped_class_t {
public:
    inventory_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(inventory_class_, "net.minecraft.world.entity.player.Inventory");
        instance_object_ = instance;
    }

    item_stack_t get_item(int index) const;
    item_stack_t get_armor(int index) const;

    inventory_t() {};
private:
    inline static std::shared_ptr<::java_class_t> inventory_class_;
};

class lang_integer_t : public mapped_class_t {
public:
    lang_integer_t(const java_object_t& instance) : mapped_class_t()
    {
        FIND_CLASS(lang_integer_class, "java.lang.Integer");
        instance_object_ = instance;
    }

    lang_integer_t() {};

    int value() const;

private:
    inline static std::shared_ptr<::java_class_t> lang_integer_class;
};

class chat_formatting_t : public mapped_class_t {
public:
    chat_formatting_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(chat_formatting_class_, "net.minecraft.ChatFormatting");
        instance_object_ = instance;

    }

    chat_formatting_t() {};

    lang_integer_t get_color() const;
private:
    inline static std::shared_ptr<::java_class_t> chat_formatting_class_;
};

class team_t : public mapped_class_t {
public:
    team_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(team_class_, "net.minecraft.world.scores.PlayerTeam");
        instance_object_ = instance;
    }

    team_t() {};

    chat_component_t player_prefix() const;
    chat_component_t player_suffix() const;
    chat_formatting_t team_color() const;
private:
    inline static std::shared_ptr<::java_class_t> team_class_;
};

class player_info_t : public mapped_class_t {
public:
    player_info_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(player_info_class_, "net.minecraft.client.multiplayer.PlayerInfo");
        instance_object_ = instance;
    }

    player_info_t() {};

    chat_component_t tab_list_display_name() const;
    team_t get_team() const;
private:
    inline static std::shared_ptr<::java_class_t> player_info_class_;
};

class entity_t : public mapped_class_t {
public:
    entity_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(player_class_, "net.minecraft.world.entity.player.Player");
        INITIALIZE_CLASS(entity_class_, "net.minecraft.world.entity.Entity");
        INITIALIZE_CLASS(living_entity_class_, "net.minecraft.world.entity.LivingEntity");
        INITIALIZE_CLASS(abstract_client_player_, "net.minecraft.client.player.AbstractClientPlayer");
        INITIALIZE_CLASS(vec3_class_, "net.minecraft.world.phys.Vec3");
        instance_object_ = instance;
    }

    entity_t() {};

    aabb_t bb() const;

    float get_health() const;

    item_stack_t get_main_hand_item() const;

    vec3<double> position() const;
    vec3<double> last_position() const;

    inventory_t inventory() const;

    chat_component_t get_display_name() const;

    std::string get_scoreboard_name() const;

    player_info_t get_player_info() const;

    void set_shared_flag(int flag, bool state) const;

private:
    inline static std::shared_ptr<::java_class_t> entity_class_;
    inline static std::shared_ptr<::java_class_t> player_class_;
    inline static std::shared_ptr<::java_class_t> living_entity_class_;
    inline static std::shared_ptr<::java_class_t> abstract_client_player_;
    inline static std::shared_ptr<::java_class_t> vec3_class_;
};

class option_instance_t : public mapped_class_t {
public:
    option_instance_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(option_instance_class_, "net.minecraft.client.OptionInstance");
        FIND_CLASS(numbers_class_, "java.lang.Number");
        FIND_CLASS(double_class_, "java.lang.Double");
        instance_object_ = instance;
    }

    float get_value() const;
    void set_value(double val) const;

    option_instance_t() {};
private:
    inline static std::shared_ptr<::java_class_t> option_instance_class_;
    inline static std::shared_ptr<::java_class_t> numbers_class_;
    inline static std::shared_ptr<::java_class_t> double_class_;
};

class key_mapping_t : public mapped_class_t {
public:
    key_mapping_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(key_mapping_class_, "net.minecraft.client.KeyMapping");
        instance_object_ = instance;
    }

    key_mapping_t() {};

    bool is_down() const;
    void set_down(bool state) const;
private:
    inline static std::shared_ptr<::java_class_t> key_mapping_class_;
};

class options_t : public mapped_class_t {
public:
    options_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(option_class_, "net.minecraft.client.Options");
        instance_object_ = instance;
    }

    options_t() {};

    option_instance_t gamma() const;
    option_instance_t fov() const;

    key_mapping_t key_sprint() const;
private:
    inline static std::shared_ptr<::java_class_t> option_class_;
};

class camera_t : public mapped_class_t {
public:
    camera_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(camera_class_, "net.minecraft.client.Camera");
        INITIALIZE_CLASS(vec3_class_, "net.minecraft.world.phys.Vec3");
        instance_object_ = instance;
    }

    camera_t() {};

    vec3<float> get_position() const;
    vec2<float> get_rotation() const;

    float get_partial_ticks() const;
    float get_eye_height() const;
private:
    inline static std::shared_ptr<::java_class_t> camera_class_;
    inline static std::shared_ptr<::java_class_t> vec3_class_;
};

class entity_render_dispatcher_t : public mapped_class_t {
public:
    entity_render_dispatcher_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(entity_render_dispatcher_class_, "net.minecraft.client.renderer.entity.EntityRenderDispatcher");
        instance_object_ = instance;
    }

    entity_render_dispatcher_t() {};

    camera_t camera() const;

private:
    inline static std::shared_ptr<::java_class_t> entity_render_dispatcher_class_;
};

class level_t : public mapped_class_t {
public :
    level_t(const java_object_t& instance) : mapped_class_t() 
    {
        INITIALIZE_CLASS(level_lass_, "net.minecraft.client.multiplayer.ClientLevel");
        instance_object_ = instance;
    }

    level_t() {};

    std::vector<entity_t*> player_list() const;

private:
    inline static std::shared_ptr<::java_class_t> level_lass_;
};

class minecraft_t : public mapped_class_t {
public:
    minecraft_t(const java_object_t& instance) : mapped_class_t()
    {
        INITIALIZE_CLASS(minecraft_class_, "net.minecraft.client.Minecraft");
        instance_object_ = instance;
    }

    minecraft_t();

    entity_t player() const;
    entity_t camera_entity() const;

    hit_result_t hit_result() const;
    screen_t screen() const;
    options_t options() const;
    level_t level() const;
    entity_render_dispatcher_t entity_render_dispatcher() const;
    resource_manager_t resource_manager() const;
private:
    inline static std::shared_ptr<::java_class_t> minecraft_class_;
};