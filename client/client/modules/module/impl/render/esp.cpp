#include "pch.hpp"
#include "modules.hpp"
#include <gl/GL.h>
#include <types/matrix.hpp>
#include <array>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "modules/helpers/helpers.hpp"

std::vector<vec4<float>> render_bds;
std::vector<aabb<double>> render_bd2s;

std::vector<float> projection;
std::vector<float> modelview;

struct player_data_t
{
	std::array<std::string, 5> armor = { };
	std::string name = {};
	std::string prefix = {};
	std::string suffix = {};
	std::string distance = {};
	std::string health = {};

	float health_native = 0.F;
	float distance_native = 0.F;

	std::vector<vec3<double>> bounds = { };
	aabb<double> aabb_bounds;

	int color = INT_MAX;
	int prefix_color = INT_MAX;
	int filled_color = 0;

	bool team_color_valid = false;
};

namespace render_data
{
	inline matrix model_view = { };
	inline matrix projection = { };

	inline vec2<float> camera_angles = { };
	inline float fov = 0.f;

	namespace esp
	{
		std::vector<player_data_t> player_data_list = { };
		std::mutex player_data_mutex = { };
	}

	namespace textures
	{
		std::unordered_map<std::string, unsigned long long> loopup_cache;

		std::unordered_map<std::string, unsigned long long> texture_map;
		std::unordered_map<std::string, std::vector<uint8_t>> texture_queue;
	}
}

std::atomic<bool> initialize_textures = true;

class font_alloc
{
public:	
	font_alloc(int index) : index_(index) {
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[index_]);
		orig_font_size_ = ImGui::GetCurrentContext()->FontSize;
	}
	
	~font_alloc() {
		ImGui::PopFont();
		ImGui::GetCurrentContext()->FontSize = orig_font_size_;
	}

private:
	int index_;
	float orig_font_size_;
};

void esp::run(JNIEnv* env, const minecraft_t& minecraft, const entity_t& player) {
	if (!minecraft || !player) {
		render_data::esp::player_data_list.clear();
		return;
	}

	static bool glow_set = false;

	if (!this->get_state()) {
		if (glow_set) {
			const auto level = minecraft.level();
			const auto player_list = level.player_list();

			for (auto& entity : player_list) {
				entity->set_shared_flag(6, false);
			}

			glow_set = false;
		}

		render_data::esp::player_data_list.clear();
		return;
	}

	const auto level = minecraft.level();
	const auto options = minecraft.options();
	const auto fov_option = options.fov();
	const float partial_tick_time = minecraft.entity_render_dispatcher().camera().get_partial_ticks();

	std::vector<player_data_t> new_player_data_list;
	const vec3 local_pos = player.position();
	const auto resource_manager = minecraft.resource_manager();

	// Reserve memory upfront
	const auto player_list = level.player_list();
	new_player_data_list.reserve(player_list.size());

	// Pre-calculate camera data
	const auto camera = minecraft.entity_render_dispatcher().camera();
	render_data::camera_angles = camera.get_rotation();
	render_data::fov = fov_option.get_value() * 1.0f;

	static bool once = false;

	if (initialize_textures)
	{
		auto textures = built_in_registries_t::item_registry().GetRegistryEntries();

		for (auto& [resource_loc, item] : textures) 
		{
			std::string description_id = item.get_description_id();
			size_t pos = description_id.find_last_of('.');
			description_id = (pos != std::string::npos) ? description_id.substr(pos + 1) : description_id;


			if (render_data::textures::texture_map.contains(description_id) || render_data::textures::texture_queue.contains(description_id))
				continue;

			resource_location_t resource_loc_t = item.texture_resource_location();

			std::vector<uint8_t> texture = resource_manager.get_resource(resource_loc_t).resource_stream();

			if (!texture.empty())
			{
				render_data::textures::texture_queue.emplace(move(description_id), move(texture));
			}
		}

		std::cout << "textures initialized -> " << render_data::textures::texture_queue.size() << std::endl;
		initialize_textures = false;
	}

	auto process_item = [](const item_t& item, const resource_manager_t& resource_manager, player_data_t& player_data, int index)
	{
		//lookup_t ez;


		if (!item.instance()) return;

		std::string description_id = item.get_description_id();

		size_t pos = description_id.find_last_of('.');
		description_id = (pos != std::string::npos) ? description_id.substr(pos + 1) : description_id;

		if (description_id == "air") {
			return;
		}

		player_data.armor[index] = description_id;

		if (render_data::textures::texture_queue.contains(description_id)) {
			return;
		}

		if (render_data::textures::texture_map.contains(description_id)) {
			return;
		}

		resource_location_t resource_loc = item.texture_resource_location();
		std::vector<uint8_t> texture = resource_manager.get_resource(resource_loc).resource_stream();

		if (!texture.empty()) {
			render_data::textures::texture_queue.emplace(move(description_id), move(texture));
		}
	};

	for (auto& entity : player_list) {
		//java_local_frame_t frame;

		if (entity->instance().is_same_object(player.instance()))
			continue;

		if (glow_outlines) {
			entity->set_shared_flag(6, true);
			glow_set = true;
		}
		else {
			entity->set_shared_flag(6, false);
			glow_set = false;
		}
		

		player_data_t player_data;
		const vec3 entity_pos = entity->position();

		player_data.distance_native = local_pos.distance(entity_pos);

		char distance_str[16];
		snprintf(distance_str, sizeof(distance_str), "%.1f", player_data.distance_native);
		player_data.distance = distance_str;

		player_data.health_native = player.get_health();

		char health_str[16];
		snprintf(health_str, sizeof(health_str), "%.1f", player_data.health_native);

		player_data.health = health_str;

		if (nametags) {
			const auto inventory = entity->inventory();

			// Process armor items
			for (int i = 0; i < 4; ++i) {
				process_item(inventory.get_armor(i).get_item(),
					resource_manager, player_data, i);
			}

			// Process main hand item
			process_item(entity->get_main_hand_item().get_item(),
				resource_manager, player_data, 4);
		}

		player_info_t player_info = entity->get_player_info();
		
		if (player_info.instance())
		{
			team_t team = player_info.get_team();

			if (team.instance())
			{
				chat_component_t prefix = team.player_prefix();

				if (prefix.instance()) {
					player_data.prefix = prefix.get_string();
				}

				chat_formatting_t color = team.team_color();

				if (color.instance()) {
					player_data.prefix_color = color.get_color().value();
				}

				player_data.name = entity->get_scoreboard_name();

				chat_component_t display_name = entity->get_display_name();

				if (display_name.instance()) {
					chat_style_t display_style = display_name.get_style();

					if (display_style.instance()) {
						player_data.color = display_style.color().value();
					}

					if (player_data.name.empty()) {
						player_data.name = display_name.get_string();
					}
				}
				// Get player name

				//player_data.prefix_color = (static_cast<int>(nametag_text_color.x * 255.f) << 16) | (static_cast<int>(nametag_text_color.y * 255.f) << 8) | static_cast<int>(nametag_text_color.z * 255.f);
				//player_data.color = (static_cast<int>(nametag_text_color.x * 255.f) << 16) | (static_cast<int>(nametag_text_color.y * 255.f) << 8) | static_cast<int>(nametag_text_color.z * 255.f);

				if (player_data.color == 0) {
					player_data.team_color_valid = false;
					player_data.color = (static_cast<int>(nametag_text_color.x * 255.f) << 16) | (static_cast<int>(nametag_text_color.y * 255.f) << 8) | static_cast<int>(nametag_text_color.z * 255.f);
				}

				if (player_data.prefix_color == 0) {
					player_data.prefix_color = (static_cast<int>(nametag_text_color.x * 255.f) << 16) | (static_cast<int>(nametag_text_color.y * 255.f) << 8) | static_cast<int>(nametag_text_color.z * 255.f);
				}
			}
		}

		// Calculate bounding box
		const auto current_pos = entity->position();
		const auto last_pos = entity->last_position();

		vec3<double> current_player_position_lerp =
		{
			std::lerp(last_pos.x, current_pos.x, partial_tick_time),
			std::lerp(last_pos.y, current_pos.y, partial_tick_time),
			std::lerp(last_pos.z, current_pos.z, partial_tick_time),
		}; 

		current_player_position_lerp.y -= 0.6;

		auto camera_position = camera.get_position();

		aabb<double> bounds = entity->bb().bounds();

		aabb aabb_bounds = entity->bb().bounds();

		aabb bounds_copy = aabb_bounds;

		aabb_bounds.max = (bounds_copy.max - bounds_copy.min) / 2.f;
		aabb_bounds.min = (bounds_copy.min - bounds_copy.max) / 2.f;

		player_data.bounds.push_back(camera_position + aabb_bounds.min - current_player_position_lerp);
		player_data.bounds.push_back(camera_position + aabb_bounds.max - current_player_position_lerp);
		player_data.bounds.push_back(camera_position + vec3<double>(aabb_bounds.min.x, aabb_bounds.min.y, aabb_bounds.max.z) - current_player_position_lerp);
		player_data.bounds.push_back(camera_position + vec3<double>(aabb_bounds.min.x, aabb_bounds.max.y, aabb_bounds.max.z) - current_player_position_lerp);
		player_data.bounds.push_back(camera_position + vec3<double>(aabb_bounds.max.x, aabb_bounds.min.y, aabb_bounds.max.z) - current_player_position_lerp);
		player_data.bounds.push_back(camera_position + vec3<double>(aabb_bounds.max.x, aabb_bounds.max.y, aabb_bounds.min.z) - current_player_position_lerp);
		player_data.bounds.push_back(camera_position + vec3<double>(aabb_bounds.max.x, aabb_bounds.min.y, aabb_bounds.min.z) - current_player_position_lerp);
		player_data.bounds.push_back(camera_position + vec3<double>(aabb_bounds.min.x, aabb_bounds.max.y, aabb_bounds.min.z) - current_player_position_lerp);

		if (render_type == RENDER_3D)
		{
			bounds_copy.min.y += 1.6;
			bounds_copy.max.y += 1.75;

			bounds_copy.min = bounds_copy.min - current_pos + (last_pos + (current_pos - last_pos) * partial_tick_time) - camera_position;
			bounds_copy.max = bounds_copy.max - current_pos + (last_pos + (current_pos - last_pos) * partial_tick_time) - camera_position;

			player_data.aabb_bounds = bounds_copy;
		}

		new_player_data_list.emplace_back(std::move(player_data));

		delete entity;
	}

	{
		//std::lock_guard<std::mutex> lock(render_data::esp::player_data_mutex);
		render_data::esp::player_data_list.swap(new_player_data_list);
	}
}

void esp::on_imgui_render()
{
	if (!this->get_state()) {
		return;
	}

	font_alloc font(1);

	vec2<float> screen_size = { ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y };

	render_data::model_view = helpers::render::caclulate_modelview(render_data::camera_angles);
	render_data::projection = helpers::render::calculate_projection(screen_size, render_data::fov);

	if (clear_texture_atlas)
	{
		/*for (auto& [key, data] : render_data::textures::texture_map) {
			stbi_image_free((void*)data);
		}*/
		render_data::textures::texture_queue.clear();
		render_data::textures::texture_map.clear();
		clear_texture_atlas = false;
		initialize_textures = true;
	}

	if (!render_data::textures::texture_queue.empty())
	{
		std::vector<std::string> processed_keys; 

		for (auto& [key, data] : render_data::textures::texture_queue)
		{
			if (unsigned long long tex = helpers::render::load_texture(data.data(), data.size()); tex != 0) {
				render_data::textures::texture_map.emplace(key, tex);
				processed_keys.push_back(key);
				std::cout << "loaded " << key << " -> " << tex << std::endl;
			}
		}

		for (const auto& key : processed_keys) {
			render_data::textures::texture_queue.erase(key);
		}
	}

	


	std::vector<player_data_t> player_data_copy = {};

	{
		player_data_copy = render_data::esp::player_data_list;
	}

	ImVec2 display_size = ImGui::GetIO().DisplaySize;
	//vec2<float> screen_size = vec2<float>(display_size.x, display_size.y);

	for (player_data_t& player_data : player_data_copy)
	{
		/*float base_font_scale = 1.0f;
		float auto_scale_modifier = 1.f;


		auto_scale_modifier = std::clamp(1.0f - (static_cast<float>(player_data.distance_native) / 100.f), 0.5f, 1.0f);

		float modified_font_size = 14.f * base_font_scale * auto_scale_modifier * 1.F;

		ImGuiContext* imgui_context = ImGui::GetCurrentContext();
		imgui_context->FontSize = modified_font_size*/;

		vec4<float> player_screen_bounds = vec4<float>(FLT_MAX, FLT_MAX, FLT_MIN, FLT_MIN);

		for (vec3<double>& current_bound : player_data.bounds)
		{
			vec2<float> current_bound_screen_position = vec2<float>();

			if (!helpers::render::world2screen(render_data::projection, render_data::model_view, screen_size, current_bound, current_bound_screen_position)) {
				continue;
			}

			player_screen_bounds.x = fminf(current_bound_screen_position.x, player_screen_bounds.x);
			player_screen_bounds.y = fminf(current_bound_screen_position.y, player_screen_bounds.y);
			player_screen_bounds.z = fmaxf(current_bound_screen_position.x, player_screen_bounds.z);
			player_screen_bounds.w = fmaxf(current_bound_screen_position.y, player_screen_bounds.w);
		}

		if (player_screen_bounds.z <= 0 || player_screen_bounds.w <= 0 || display_size.x < player_screen_bounds.x || display_size.y < player_screen_bounds.y) {
			continue;
		}

		ImRect player_bounds = ImRect(ImVec2(player_screen_bounds.x, player_screen_bounds.y), ImVec2(player_screen_bounds.z, player_screen_bounds.w));

		int name_color_r = (player_data.color >> 16) & 0xFF;
		int name_color_g = (player_data.color >> 8) & 0xFF;
		int name_color_b = player_data.color & 0xFF;

		ImColor name_color(name_color_r, name_color_g, name_color_b);

		if (render_type == RENDER_2D)
		{
			if (fill) {
				ImGui::GetBackgroundDrawList()->AddRectFilled(player_bounds.Min, player_bounds.Max, color(fill_color));
			}

			if (outline) {
				ImGui::GetBackgroundDrawList()->AddRect(player_bounds.Min, player_bounds.Max, color(outline_color));
			}
		}

		if (render_type == RENDER_3D)
		{
			std::vector<float> model_view_vector = render_data::model_view.to_vector();
			std::vector<float> projection_vector = render_data::projection.to_vector();

			glPushMatrix();
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(projection_vector.data());
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(model_view_vector.data());

			glViewport(0, 0, static_cast<int>(screen_size.x), static_cast<int>(screen_size.y));

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_LINE_SMOOTH);
			glLineWidth(esp_outline_thickness);

			if (fill)
			{
				glColor4f(fill_color.x, fill_color.y, fill_color.z, fill_color.w);

				glBegin(GL_QUADS);

				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.min.z);

				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.max.z);

				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.min.z);

				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.min.z);

				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.max.z);

				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.max.z);

				glEnd();
			}

			if (match_esp_outlines_with_team_color && player_data.team_color_valid) {
				glColor4f(name_color_r / 255.F, name_color_g / 255.F, name_color_b / 255.F, outline_color.w);
			}
			else {
				glColor4f(outline_color.x, outline_color.y, outline_color.z, outline_color.w);
			}
			
			if (outline)
			{
				glBegin(GL_LINES);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.max.z);
				glEnd();

				glBegin(GL_LINE_STRIP);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.max.y, player_data.aabb_bounds.min.z);
				glEnd();

				glBegin(GL_LINE_STRIP);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.min.z);
				glVertex3d(player_data.aabb_bounds.max.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.max.z);
				glVertex3d(player_data.aabb_bounds.min.x, player_data.aabb_bounds.min.y, player_data.aabb_bounds.min.z);
				glEnd();

				glDisable(GL_LINE_SMOOTH);
				glDisable(GL_BLEND);
			}

			glPopMatrix();
		}

		if (nametags)
		{
			ImVec2 name_size = {};
			player_data.prefix.erase(std::remove_if(player_data.prefix.begin(), player_data.prefix.end(), ::isspace), player_data.prefix.end());
			
			if (!player_data.prefix.empty()) {
				name_size.x += ImGui::CalcTextSize((player_data.prefix).c_str()).x + whitespace_spacing;
			}

			name_size += ImGui::CalcTextSize((player_data.name).c_str()) + ImVec2(whitespace_spacing, 0.F);

			if (render_brackets) {
				name_size.x += ImGui::CalcTextSize("[").x + whitespace_spacing;
				name_size.x += ImGui::CalcTextSize("]").x + whitespace_spacing;
				name_size.x += ImGui::CalcTextSize("[").x + whitespace_spacing;
				name_size.x += ImGui::CalcTextSize("]").x + whitespace_spacing;
			}

			if (render_health) {
				name_size.x += ImGui::CalcTextSize((player_data.health).c_str()).x + (render_distance ? whitespace_spacing : 0);
			}

			if (render_distance) {
				name_size.x += ImGui::CalcTextSize((player_data.distance).c_str()).x;
			}

			ImRect nametag_bound = ImRect(ImVec2(player_bounds.GetCenter().x - ((name_size.x / 2.F) + x_padding), player_bounds.Min.y - y_spacing - (name_size.y) - y_padding), ImVec2(player_bounds.GetCenter().x + ((name_size.x / 2.F) + x_padding), player_bounds.Min.y - y_spacing));
			
			if (render_nametag_background)
			{
				ImGui::GetBackgroundDrawList()->AddRectFilled(nametag_bound.Min, nametag_bound.Max, color(nametag_fill_color));
				ImGui::GetBackgroundDrawList()->AddRect(nametag_bound.Min, nametag_bound.Max, color(nametag_outline_color));
			}

			int prefix_color_r = (player_data.prefix_color >> 16) & 0xFF;
			int prefix_color_g = (player_data.prefix_color >> 8) & 0xFF;
			int prefix_color_b = player_data.prefix_color & 0xFF;

			ImColor prefix_color(prefix_color_r, prefix_color_g, prefix_color_b);

			float x_pos = nametag_bound.Min.x + x_padding;

			float health_percentage = player_data.health_native / 20.F;

			ImColor health_color = {
				(int)std::lerp(168, 0, health_percentage),
				(int)std::lerp(0, 170, health_percentage),
				(int)std::lerp(0, 0, health_percentage),
				255
			};

			if (!player_data.prefix.empty())
			{
				ImGui::GetBackgroundDrawList()->AddText(ImVec2{ x_pos, nametag_bound.GetCenter().y - (name_size.y / 2) }, prefix_color, player_data.prefix.c_str());
				x_pos += ImGui::CalcTextSize((player_data.prefix).c_str()).x + whitespace_spacing;
			}


			ImGui::GetBackgroundDrawList()->AddText(ImVec2{ x_pos, nametag_bound.GetCenter().y - (name_size.y / 2) }, name_color, player_data.name.c_str());
			x_pos += ImGui::CalcTextSize((player_data.name).c_str()).x + whitespace_spacing;

			/*if (!player_data.suffix.empty())
			{
				ImGui::GetBackgroundDrawList()->AddText(ImVec2{ x_pos, nametag_bound.GetCenter().y - (name_size.y / 2) }, prefix_color, player_data.suffix.c_str());
				x_pos += ImGui::CalcTextSize((player_data.suffix + " ").c_str()).x;
			}*/
			if (render_brackets) {
				ImGui::GetBackgroundDrawList()->AddText(ImVec2{ x_pos, nametag_bound.GetCenter().y - (name_size.y / 2) }, color(nametag_bracket_color), "[");
				x_pos += ImGui::CalcTextSize("[").x + whitespace_spacing;
			}

			if (render_health) {
				ImGui::GetBackgroundDrawList()->AddText(ImVec2{ x_pos, nametag_bound.GetCenter().y - (name_size.y / 2) }, dynamic_health_color ? color(health_color) : color(nametag_health_color), player_data.health.c_str());
				x_pos += ImGui::CalcTextSize((player_data.health).c_str()).x + whitespace_spacing;
			}

			if (render_brackets) {
				ImGui::GetBackgroundDrawList()->AddText(ImVec2{ x_pos, nametag_bound.GetCenter().y - (name_size.y / 2) }, color(nametag_bracket_color), "]");
				x_pos += ImGui::CalcTextSize("]").x + whitespace_spacing;
			}

			if (render_brackets){
				ImGui::GetBackgroundDrawList()->AddText(ImVec2{ x_pos, nametag_bound.GetCenter().y - (name_size.y / 2) }, color(nametag_bracket_color), "[");
				x_pos += ImGui::CalcTextSize("[").x + whitespace_spacing;
			}

			if (render_distance) {
				ImGui::GetBackgroundDrawList()->AddText(ImVec2{ x_pos, nametag_bound.GetCenter().y - (name_size.y / 2) }, color(nametag_distance_color), player_data.distance.c_str());
				x_pos += ImGui::CalcTextSize((player_data.distance).c_str()).x + whitespace_spacing;
			}

			if (render_brackets) {
				ImGui::GetBackgroundDrawList()->AddText(ImVec2{ x_pos, nametag_bound.GetCenter().y - (name_size.y / 2) }, color(nametag_bracket_color), "]");
				x_pos += ImGui::CalcTextSize("]").x + whitespace_spacing;
			}
			/*ImGui::GetBackgroundDrawList()->AddText(ImVec2{ nametag_bound.Min.x + x_padding, nametag_bound.GetCenter().y - (name_size.y / 2) } + ImVec2(ImGui::CalcTextSize((player_data.name + " ").c_str()).x, 0), ImGui::GetColorU32(ImVec4(0.60f, 0.60f, 0.60f, 1.00f)), "[");
			ImGui::GetBackgroundDrawList()->AddText(ImVec2{ nametag_bound.Min.x + x_padding, nametag_bound.GetCenter().y - (name_size.y / 2) } + ImVec2(ImGui::CalcTextSize((player_data.name + " [ ").c_str()).x, 0), IM_COL32_WHITE, player_data.distance.c_str());
			ImGui::GetBackgroundDrawList()->AddText(ImVec2{ nametag_bound.Min.x + x_padding, nametag_bound.GetCenter().y - (name_size.y / 2) } + ImVec2(ImGui::CalcTextSize((player_data.name + " [ " + player_data.distance + " ").c_str()).x, 0), ImGui::GetColorU32(ImVec4(0.60f, 0.60f, 0.60f, 1.00f)), "]");*/

			if (render_equippment)
			{
				int count = 0;
				std::vector<unsigned long long> textures;

				for (int i = 4; i >= 0; i--) {
					if (render_data::textures::texture_map.contains(player_data.armor[i])) {
						unsigned long long texture_id = render_data::textures::texture_map.at(player_data.armor[i]);
						if (texture_id != 0) {
							textures.push_back(texture_id);
						}
					}
				}

				count = static_cast<int>(textures.size());

				if (count == 0) continue;

				float total_width = count * texture_size + (count - 1) * texture_spacing;

				ImVec2 start_pos = ImVec2(player_bounds.GetCenter().x - (total_width / 2.F),
					nametag_bound.Min.y - y_spacing - texture_size);

				for (int i = 0; i < count; i++) {
					ImVec2 pos = ImVec2(start_pos.x + i * (texture_size + texture_spacing), start_pos.y);
					ImVec2 size = ImVec2(pos.x + texture_size, pos.y + texture_size);

					ImGui::GetBackgroundDrawList()->AddImage(reinterpret_cast<void*>(textures[i]), pos, size, ImVec2(0, 0), ImVec2(1, 1));
				}
			}
		}
	}
}

void esp::on_interface_render()
{
	ImGui::Begin("esp", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	{
		ImGui::Checkbox("enabled", this->get_toggle_pointer());

		ImGui::Checkbox("render nametag", &nametags);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("nametag options");
	/*	inline static bool render_distance = true;
		inline static bool render_prefixes = true;
		inline static bool render_equippment = true;*/

		ImGui::Checkbox("render distance", &render_distance);
		ImGui::Checkbox("render brackets", &render_brackets);
		ImGui::Checkbox("render health", &render_health);
		ImGui::Checkbox("render equippment", &render_equippment);
		ImGui::Checkbox("render prefixes", &render_prefixes);
		ImGui::Checkbox("render prefixes", &render_nametag_background);
		ImGui::Checkbox("dynamic health color", &dynamic_health_color);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("esp render options");

		ImGui::Checkbox("fille", &fill);
		ImGui::Checkbox("outlines", &outline);
		ImGui::Checkbox("glow outlines", &glow_outlines);
		ImGui::Checkbox("recolor outlines", &match_esp_outlines_with_team_color);

		ImGui::Text("render mode");
		ImGui::Combo("##rendermode", &render_type, "render 2d\0render 3d");
		ImGui::SliderFloat("esp line thickness", &esp_outline_thickness, 0.F, 5.F);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("render colors");

		ImGui::ColorEdit4("fill color", (float*)&fill_color);
		ImGui::ColorEdit4("outline color", (float*)&outline_color);

		ImGui::ColorEdit4("nametag fill color", (float*)&nametag_fill_color);
		ImGui::ColorEdit4("nametag outline color", (float*)&nametag_outline_color);
		ImGui::ColorEdit4("nametag text color", (float*)&nametag_text_color);
		ImGui::ColorEdit4("nametag health color", (float*)&nametag_health_color);
		ImGui::ColorEdit4("nametag distance color", (float*)&nametag_distance_color);
		ImGui::ColorEdit4("nametag bracket color", (float*)&nametag_bracket_color);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("alignment/rendering");

		ImGui::SliderFloat("y padding", &y_padding, 0.F, 10.F);
		ImGui::SliderFloat("x padding", &x_padding, 0.F, 10.F);
		ImGui::SliderFloat("y spacing", &y_spacing, 0.F, 10.F);
		ImGui::SliderFloat("texture spacing", &texture_spacing, 0.F, 10.F);
		ImGui::SliderFloat("texture size", &texture_size, 0.F, 20.F);
		ImGui::SliderFloat("whitespace spacing", &whitespace_spacing, 0.F, 20.F);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("experimental");

		ImGui::Checkbox("flip textures", &flip_textures);
		ImGui::Checkbox("clear texture atlas", &clear_texture_atlas);


		static helpers::gui::keybind bind;
		helpers::gui::key_bind_button("bind", &bind);
		this->set_bind(bind.vk_code);
	}
	ImGui::End();
}

void esp::destroy()
{

}
