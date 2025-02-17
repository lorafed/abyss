#pragma once
#include "module/module.hpp"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "types/color.hpp"

#include "modules/helpers/helpers.hpp"

class left_clicker : public module
{
public:
	left_clicker() : module("Left Clicker", COMBAT) {};

	inline static vec2<int> cps = { 10, 16 };

	inline static int break_blocks_delay = 50;

	inline static bool break_blocks = true;
	inline static bool whitelist_inventories = true;
	inline static bool require_mouse_down = true;
	inline static bool whitelist_weapons = false;
	inline static bool no_randomization = false;

	inline static bool allow_swords = true;
	inline static bool allow_axes = true;

private:
	virtual void run(JNIEnv* env, const minecraft_t& minecraft, const entity_t& player) override;
	virtual void on_interface_render() override;
};

class right_clicker : public module
{
public:
	right_clicker() : module("Right Clicker", UTILITY) {};

	inline static vec2<int> cps = { 10, 16 };

	inline static bool whitelist_inventories = true;
	inline static bool require_mouse_down = true;
	inline static bool blocks_only = true;

private:
	virtual void run(JNIEnv* env, const minecraft_t& minecraft, const entity_t& player) override;
	virtual void on_interface_render() override;
};

class full_bright : public module
{
public:
	full_bright() : module("Full Bright", RENDER) {};

	inline static float brightness = 1.;
private:
	virtual void run(JNIEnv* env, const minecraft_t& minecraft, const entity_t& player) override;
	virtual void on_interface_render() override;
};

class esp : public module
{
public:
	esp() : module("ESP", RENDER) {};

	enum RENDER_TYPE
	{
		RENDER_2D = 0,
		RENDER_3D = 1,
	};

	enum NAMETAG_STYLE
	{
		DEFAULT = 0,
		VAPE = 1
	};

	inline static int render_type = RENDER_2D;
	inline static bool nametags = false;
	inline static bool fill = true;
	inline static bool outline = true;
	inline static bool glow_outlines = true;

	inline static bool recolor_visuals = true;

	inline static bool flip_textures = false;

	inline static bool render_distance = true;
	inline static bool render_health = true;
	inline static bool render_prefixes = true;
	inline static bool render_equippment = true;

	inline static bool render_nametag_background = true;
	inline static bool dynamic_health_color = true;
	inline static bool render_brackets = true;

	inline static bool match_esp_outlines_with_team_color = true;

	inline static bool clear_texture_atlas = false;

	inline static float y_padding = 5.F;
	inline static float x_padding = 5.F;
	inline static float y_spacing = 5.F;
	inline static float texture_spacing = 2.F;
	inline static float whitespace_spacing = 6.F;
	inline static float texture_size = 15.F;
	inline static float esp_outline_thickness = 1.F;

	inline static ImVec4 outline_color = color(255, 255, 255, 125); // Default to white color (RGBA)
	inline static ImVec4 fill_color = color(0, 0, 0); // Default to white color (RGBA)

	inline static ImVec4 nametag_fill_color = color(0, 0, 0); // Default to white color (RGBA)
	inline static ImVec4 nametag_outline_color = color(0, 0, 255); // Default to white color (RGBA)
	inline static ImVec4 nametag_text_color = color(255, 255, 255); // Default to white color (RGBA)
	inline static ImVec4 nametag_health_color = color(255, 255, 255); // Default to white color (RGBA
	inline static ImVec4 nametag_distance_color = color(0, 170, 0); // Default to white color (RGBA)
	inline static ImVec4 nametag_bracket_color = color(168, 168, 168); // Default to white color (RGBA)
private:
	virtual void run(JNIEnv* env, const minecraft_t& minecraft, const entity_t& player) override;
	virtual void on_interface_render() override;
	virtual void on_imgui_render() override;
	virtual void destroy() override;
};

class nametags : public module
{
public:
	nametags() : module("Nametags", RENDER) {};
private:
	virtual void run(JNIEnv* env, const minecraft_t& minecraft, const entity_t& player) override;
	virtual void on_interface_render() override;
	virtual void on_imgui_render() override;
};

class sprint : public module
{
public:
	sprint() : module("Sprint", MOVEMENT) {};
private:
	virtual void run(JNIEnv* env, const minecraft_t& minecraft, const entity_t& player) override;
	virtual void on_interface_render() override;
};

class modules
{
public:
	static modules* get_instance();
	static void initialize();
	static void native_run();
	static void destroy();

	static module* module_by_name(const std::string& name);
	static void disable_all();
	static std::vector<::module*> get_modules();
private:
	inline static std::thread main_thread_;
	inline static std::thread render_thread_;
	inline static modules* instance_;
	inline static std::vector<::module*> modules_;
};