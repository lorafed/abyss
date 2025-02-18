#include "exceptions.hpp"
#include "sdk.hpp"

clipboard_exception::clipboard_exception(const std::string& message, const std::string& file, int line)
{
	std::string log_message = message + "\nfile: " + file + "\nline: " + std::to_string(line);
	windows::set_clipboard_text(log_message);
}