#include "pch.hpp"
#include "helpers.hpp"

#include "imgui/imgui.h"

int helpers::gui::imgui_vk(ImGuiKey key) {

	if (key >= ImGuiKey_A && key <= ImGuiKey_Z)
		return 'A' + (key - ImGuiKey_A);

	if (key >= ImGuiKey_0 && key <= ImGuiKey_9)
		return '0' + (key - ImGuiKey_0);

	switch (key) {
	case ImGuiKey_Space: return VK_SPACE;
	case ImGuiKey_Enter: return VK_RETURN;
	case ImGuiKey_Escape: return VK_ESCAPE;
	case ImGuiKey_Backspace: return VK_BACK;
	case ImGuiKey_Tab: return VK_TAB;
	case ImGuiKey_LeftArrow: return VK_LEFT;
	case ImGuiKey_RightArrow: return VK_RIGHT;
	case ImGuiKey_UpArrow: return VK_UP;
	case ImGuiKey_DownArrow: return VK_DOWN;
	case ImGuiKey_Insert: return VK_INSERT;
	case ImGuiKey_Delete: return VK_DELETE;
	case ImGuiKey_Home: return VK_HOME;
	case ImGuiKey_End: return VK_END;
	case ImGuiKey_PageUp: return VK_PRIOR;
	case ImGuiKey_PageDown: return VK_NEXT;
	case ImGuiKey_CapsLock: return VK_CAPITAL;
	case ImGuiKey_ScrollLock: return VK_SCROLL;
	case ImGuiKey_NumLock: return VK_NUMLOCK;
	case ImGuiKey_PrintScreen: return VK_SNAPSHOT;
	case ImGuiKey_Pause: return VK_PAUSE;
	case ImGuiKey_F1: return VK_F1;
	case ImGuiKey_F2: return VK_F2;
	default: return 0;
	}
}

const char* helpers::gui::get_vk_name(int vk) {
	if (vk == 0) return "None";

	static char name[64];
	UINT scan_code = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);

	// extended keys
	switch (vk) {
	case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
	case VK_INSERT: case VK_DELETE: case VK_HOME: case VK_END:
	case VK_PRIOR: case VK_NEXT:
	case VK_DIVIDE:
	case VK_NUMLOCK:
		scan_code |= 0x100;  // extended bit
		break;
	}

	if (GetKeyNameTextA(scan_code << 16, name, sizeof(name)) != 0)
		return name;

	return "Unknown";
}

bool helpers::gui::key_bind_button(const char* label, keybind* bind) {
	bool changed = false;
	ImGui::PushID(label);

	if (bind->is_waiting) {
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
	}

	if (ImGui::Button(label, ImVec2(100, 0))) {
		bind->is_waiting = true;
	}

	if (bind->is_waiting) {
		ImGui::PopStyleColor();

		for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key++) {
			ImGuiKey imgui_key = static_cast<ImGuiKey>(key);

			if (imgui_key >= ImGuiKey_ModCtrl && imgui_key <= ImGuiKey_ModSuper)
				continue;

			if (ImGui::IsKeyPressed(imgui_key)) {
				bind->vk_code = imgui_vk(imgui_key);
				bind->is_waiting = false;
				changed = true;
				break;
			}
		}

		if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
			bind->is_waiting = false;
		}
	}

	ImGui::SameLine();
	ImGui::Text("%s: %s", label, get_vk_name(bind->vk_code));

	ImGui::PopID();
	return changed;
}
