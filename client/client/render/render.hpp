#pragma once
#include <wtypes.h>

namespace render
{
	inline int viewport[4];

	inline void* original_wndproc = nullptr;
	inline void* original_swap_buffers = nullptr;
	inline void* main_window = nullptr;

	inline bool render_interface = true;

	void initiaize();
	void render();
	void destroy();

	bool gl_swap_buffers(HDC__* context);
	__int64 __stdcall wndproc(HWND handle, UINT msg, WPARAM w_p , LPARAM l_p);
}