#include "pch.hpp"
#include "render.hpp"

#include "ext/minhook/MinHook.h"

#include "ext/imgui/imgui_impl_win32.h"
#include "ext/imgui/imgui_impl_opengl3.h"

#include "context.hpp"

#include "modules.hpp"

void render::initiaize()
{
	MH_Initialize();

	void* wgl_swap_buffers = GetProcAddress(GetModuleHandleA("opengl32.dll"), "wglSwapBuffers");

	MH_CreateHook(wgl_swap_buffers, gl_swap_buffers, reinterpret_cast<void**>(&original_swap_buffers));
	MH_EnableHook(wgl_swap_buffers);
}

void render::render()
{
	ImGui::Begin("settings");
	{
		if (ImGui::Button("unload")) {
			ctx.unload.store(true);
		}

		static helpers::gui::keybind ui_bind{
			VK_INSERT,
			false
		};

		helpers::gui::key_bind_button("ui bind", &ui_bind);
		ctx.interface_bind = ui_bind.vk_code;
	}
	ImGui::End();

	for (auto module_ : modules::get_modules()) {
		module_->on_interface_render();
	}
}

void render::destroy()
{
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	SetWindowLongPtrA((HWND)ctx.main_window, GWLP_WNDPROC, reinterpret_cast<long long>(render::original_wndproc));

	MH_Uninitialize();
}
 