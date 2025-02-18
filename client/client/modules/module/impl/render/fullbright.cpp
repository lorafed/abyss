#include "pch.hpp"
#include "modules.hpp"

void full_bright::run(JNIEnv* env, const minecraft_t& minecraft, const entity_t& player)
{
	static bool set = false;

	if (!minecraft || !player) {
		return;
	}

	if (!this->get_state())
	{
		if (set) 
		{
			options_t options = minecraft.options();
			option_instance_t gamma = options.gamma();

			gamma.set_value(0.);
			set = false;
		}

		return;
	}

	options_t options = minecraft.options();
	option_instance_t gamma = options.gamma();

	gamma.set_value(brightness);
	set = true;
}

void full_bright::on_interface_render()
{
    ImGui::Begin("full bright", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    {
        ImGui::Checkbox("enabled", this->get_toggle_pointer());

		ImGui::SliderFloat("brightness", &brightness, 0.F, 1.F);

		static helpers::gui::keybind bind;
		helpers::gui::key_bind_button("bind", &bind);
		this->set_bind(bind.vk_code);
    }
    ImGui::End();
}
