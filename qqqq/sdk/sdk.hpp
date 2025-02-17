#pragma once

#include <string>
#include <vector>
#include <Windows.h>

#include "skcrypt.hpp"
#include "lazy_importer.hpp"

namespace windows
{
	void allocate_console();
	void deallocate_console();
	void set_clipboard_text(const std::string text);

	std::string read_file(const std::string& path);
	std::string get_appdata_path();
	std::string get_main_window_title();

	HWND get_main_window();

	bool is_key_down(__int64 key);
	bool is_key_down_toggle(__int64 key);

	bool is_window_active(void* hwnd);
}

namespace string
{
	bool compare(const std::string& str1, const std::string& str2);
	bool contains(const std::string& str, const std::string& substring);

	std::string to_lowercase(const std::string& input);
	std::string to_uppercase(const std::string& input);
	std::string trim(const std::string& input);

	std::vector<std::string> split(const std::string& str, char delimiter);
	std::string join(const std::vector<std::string>& elements, const std::string& delimiter);
}