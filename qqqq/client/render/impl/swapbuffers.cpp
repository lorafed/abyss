#include "pch.hpp"
#include "render/render.hpp"

#include "ext/imgui/imgui_impl_win32.h"
#include "ext/imgui/imgui_impl_opengl3.h"

#include "modules.hpp"

#include "context.hpp"
#include "fonts/poppins.hpp"

#include <gl/GL.h>

#include "ext/font.hpp"

bool render::gl_swap_buffers(HDC__* context)
{
	static bool initialized = false;
	static bool destroyed = false;

	void* window = WindowFromDC((HDC)context);

	if (!initialized)
	{
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(WindowFromDC((HDC)context));
		ImGui_ImplOpenGL3_Init();

		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->AddFontDefault();

		ImFontConfig font_config = { };
		font_config.FontDataOwnedByAtlas = false;

		io.Fonts->AddFontFromMemoryTTF((void*)minecraft_regular, sizeof(minecraft_regular), 14.f, &font_config);

		ctx.main_window = window;
		initialized = true;

		render::original_wndproc = reinterpret_cast<decltype(&render::wndproc)>(SetWindowLongPtrA((HWND)window, -4, (long long)render::wndproc));
	}

	if (ctx.main_window != window)
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplWin32_Shutdown();

		ImGui_ImplWin32_Init(window);
		ImGui_ImplOpenGL3_Init();

		ctx.main_window = window;

		SetWindowLongPtrA((HWND)window, GWLP_WNDPROC, reinterpret_cast<long long>(render::original_wndproc));
		render::original_wndproc = reinterpret_cast<decltype(&render::wndproc)>(SetWindowLongPtrA((HWND)window, -4, (long long)render::wndproc));
	}

	glGetIntegerv(GL_VIEWPORT, render::viewport);

	auto& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(render::viewport[2]), static_cast<float>(render::viewport[3]));
	
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	for (auto& module_ : modules::get_modules()) 
	{
		module_->on_imgui_render();
		module_->on_gl_render();
	}

	if (render::render_interface) {
		render::render();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	return ((__int32(__stdcall*)(HDC__*))render::original_swap_buffers)(context);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

__int64 __stdcall render::wndproc(HWND handle, UINT msg, WPARAM w_p, LPARAM l_p)
{
	static POINT cursor_cache{};

	if (GetActiveWindow() != handle) {
		if (render::render_interface) {
			render::render_interface = false;
		}

		return CallWindowProcA(reinterpret_cast<WNDPROC>(render::original_wndproc), handle, msg, w_p, l_p);
	}

	if (msg == WM_KEYDOWN)
	{
		if (w_p == ctx.interface_bind) {
			render::render_interface = !render::render_interface;

			if (render::render_interface) {
				GetCursorPos(&cursor_cache);
			}

			if (!render::render_interface) {
				SetCursorPos(cursor_cache.x, cursor_cache.y);
			}
		}

		for (auto& module : modules::get_modules()) {
			if (w_p == module->get_bind()) {
				module->update_state();
			}
		}
	}



	if (!render::render_interface) {
		return CallWindowProcA(reinterpret_cast<WNDPROC>(render::original_wndproc), handle, msg, w_p, l_p);
	}

	ImGui_ImplWin32_WndProcHandler(handle, msg, w_p, l_p);

	if (msg == 256 || msg == 260) {
		return false;
	}

	if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_LBUTTONUP || msg == WM_RBUTTONUP || msg == WM_LBUTTONDBLCLK || msg == WM_RBUTTONDBLCLK || msg == WM_MOUSEFIRST || msg == WM_MOUSEWHEEL) {
		return false;
	}

	return true;
}
