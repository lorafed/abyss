#include "pch.hpp"
#include "modules.hpp"

void left_clicker::run(JNIEnv* env, const minecraft_t& minecraft, const entity_t& player)
{
    if (!this->get_state()) {
        return;
    }

    static c_timer clicker_timer = c_timer(1000 / cps.y);
    static c_timer break_blocks_timer = c_timer(50);

    if (!minecraft || !player || render::render_interface) {
        return clicker_timer.reset();
    }

    if (require_mouse_down && !windows::is_key_down(VK_LBUTTON)) {
        return clicker_timer.reset();
    }

    if (!windows::is_window_active(ctx.main_window)) {
        return clicker_timer.reset();
    }

    hit_result_t raytrace = minecraft.hit_result();
    screen_t current_screen = minecraft.screen();

    if (current_screen.is_inventory())
    {
        if (whitelist_inventories)
        {
            if (!windows::is_key_down(VK_LSHIFT)) {
                return clicker_timer.reset();
            }
        }
        else {
            return clicker_timer.reset();
        }
    }
    else if (current_screen) {
        return clicker_timer.reset();
    }

    static UINT last_message_sent = WM_LBUTTONUP;

    if (break_blocks && !current_screen)
    {
        static bool is_aiming_block = false;
        if (raytrace.is_block() && !is_aiming_block)
        {
            is_aiming_block = true;
            break_blocks_timer.set_next_delay(break_blocks_delay);
        }

        if (!raytrace.is_block() && is_aiming_block) {
            is_aiming_block = false;
        }

        if (is_aiming_block && break_blocks_timer.has_elapsed())
        {
            if (last_message_sent == WM_LBUTTONUP) {
                PostMessageA((HWND)ctx.main_window, (last_message_sent = (last_message_sent == WM_LBUTTONUP ? WM_LBUTTONDOWN : WM_LBUTTONUP)), VK_LBUTTON, 0);
            }

            return clicker_timer.reset();
        }
    }

    item_t main_hand_item = player.get_main_hand_item().get_item();

    if (whitelist_weapons)
    {
        bool whitelisted = false;
        
        if (!whitelisted && allow_axes && main_hand_item.is_axe()) {
            whitelisted = true;
        }

        if (!whitelisted && allow_swords && main_hand_item.is_sword()) {
            whitelisted = true;
        }

        if (!whitelisted) {
            return clicker_timer.reset();
        }
    }

    auto get_delay = [&]() -> DWORD
    {
        int delay_min = 1000 / cps.x;
        int delay_max = 1000 / cps.y;

        static std::random_device rd;
        static std::mt19937 gen(rd());

        std::uniform_int_distribution rand(delay_max, delay_min);

        return no_randomization ? (delay_max) / 2 : rand(gen) / 2;
    };

    if (clicker_timer.has_elapsed())
    {
        PostMessageA((HWND)ctx.main_window, (last_message_sent = (last_message_sent == WM_LBUTTONUP ? WM_LBUTTONDOWN : WM_LBUTTONUP)), VK_LBUTTON, 0);
        clicker_timer.set_next_delay(get_delay());
    }
}

void left_clicker::on_interface_render()
{
    ImGui::Begin("left clicker", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    {
        ImGui::Checkbox("enabled", this->get_toggle_pointer());

        ImGui::DragIntRange2("clicks per second", &cps.x, &cps.y, 1.F, 1.F, 20.F);

        if (break_blocks) {
            ImGui::SliderInt("break blocks delay", &break_blocks_delay, 0, 250);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Checkbox("break blocks", &break_blocks);
        ImGui::Checkbox("whitelist inventories", &whitelist_inventories);
        ImGui::Checkbox("require mouse down", &require_mouse_down);
        ImGui::Checkbox("whitelist weapons", &whitelist_weapons);
        ImGui::Checkbox("disable randomization", &no_randomization);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("whitelist selection");

        ImGui::Checkbox("allow axes", &allow_axes);
        ImGui::Checkbox("allow swords", &allow_swords);

        static helpers::gui::keybind bind;
        helpers::gui::key_bind_button("bind", &bind);
        this->set_bind(bind.vk_code);
    }
    ImGui::End();
}
