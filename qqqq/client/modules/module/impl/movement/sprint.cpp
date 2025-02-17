#include "pch.hpp"
#include "modules.hpp"

void sprint::run(JNIEnv* env, const minecraft_t& minecraft, const entity_t& player)
{
	static bool set_ = false;

	if (!this->get_state()) 
	{
		if (set_)
		{
			options_t options = minecraft.options();
			key_mapping_t key_sprint = options.key_sprint();

			key_sprint.set_down(false);
			set_ = false;
		}

		return;
	}

	if (!minecraft || !player) {
		return;
	}

	options_t options = minecraft.options();
	key_mapping_t key_sprint = options.key_sprint();

	key_sprint.set_down(true);
	set_ = true;
}

void sprint::on_interface_render()
{
	ImGui::Begin("sprint", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	{
		ImGui::Checkbox("enabled", this->get_toggle_pointer());
		static helpers::gui::keybind bind;
		helpers::gui::key_bind_button("bind", &bind);
		this->set_bind(bind.vk_code);
	}
	ImGui::End();
}