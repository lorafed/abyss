#include "exceptions.hpp"
#include "sdk.hpp"

message_exception::message_exception(const std::string& message, const std::string& file, int line)
{
	std::string log_message = message + "\nfile: " + file + "\nline: " + std::to_string(line);
	MessageBox(nullptr, log_message.c_str(), "exception occurred", MB_ICONERROR | MB_OK);
}