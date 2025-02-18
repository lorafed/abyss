#pragma once

#include "types/vec2.hpp"
#include "types/vec3.hpp"
#include "types/vec4.hpp"
#include "types/c_timer.hpp"

#include "imgui/imgui.h"

#include "context/g_context.hpp"

#include "render/render.hpp"

#define UNDEFINED -1
#define COMBAT 0
#define MOVEMENT 1
#define RENDER 2
#define UTILITY 3
#define CLIENT 4

class modules;

class module
{
protected:
    int key_bind = 0;
    bool enabled = false;

    std::string name;
    int category = UNDEFINED;

    modules* manager;
public:
    module(const char* name, int category) : name(name), category(category) {};

    virtual void initialize() {}
    virtual void destroy() {}

    virtual void save_config() {}
    virtual void load_config() {}

    virtual void on_enable() {}
    virtual void on_disable() {}

    virtual void run(JNIEnv* env, const minecraft_t& minecraft, const entity_t& player) {};

    virtual void on_interface_render() {}
    virtual void on_imgui_render() {}
    virtual void on_gl_render() {}

    void set_module_manager(modules* mgr) { manager = mgr; }

    void update_state() {
        this->enabled = !this->enabled;
    }

    void enable() {
        this->enabled = true;
    }

    void disable() {
        this->enabled = false;
    }

    bool* get_toggle_pointer() {
        return &this->enabled;
    }

    int* get_bind_pointer() {
        return &this->key_bind;
    }

    void set_bind(const int bind) {
        this->key_bind = bind;
    }

    int get_bind() const {
        return this->key_bind;
    }

    bool get_state() const {
        return this->enabled;
    }

    std::string get_name() const {
        return this->name;
    }
};