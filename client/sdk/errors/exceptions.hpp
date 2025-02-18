#pragma once
#include <iostream>
#include <stdexcept>
#include <string>
#include <windows.h>

#include "ext/skcrypt.hpp"

class message_exception : public std::exception
{
private:
public:
	explicit message_exception(const std::string& message, const std::string& file, int line);
};

class clipboard_exception : public std::exception
{
private:
public:
	explicit clipboard_exception(const std::string& message, const std::string& file, int line);
}; 

//change these depending on debug or nah
#define THROW_MESSAGE_EXCEPTION(msg) throw message_exception(std_crypt(msg), std::filesystem::path(std_crypt(__FILE__)).filename().string(), __LINE__)
#define THROW_UNEXPECTED_EXCEPTION() throw message_exception(std_crypt("unexpected exception"), std::filesystem::path(std_crypt(__FILE__)).filename().string(), __LINE__)
#define THROW_CLIPBOARD_EXCEPTION(msg) throw clipboard_exception(std_crypt(msg), std::filesystem::path(std_crypt(__FILE__)).filename().string(), __LINE__)

#define EXIT_EXCEPTION \
{{ \
	exit(0); \
}}

#ifndef DEBUG_BUILD
#define EXCEPTION(msg) throw message_exception(std_crypt(msg), std::filesystem::path(std_crypt(__FILE__)).filename().string(), __LINE__)
#define EXCEPTION_NOCRYPT(msg) throw message_exception(msg, std::filesystem::path(std_crypt(__FILE__)).filename().string(), __LINE__)
#endif

#ifdef DEBUG_BUILD
#define EXCEPTION(msg) EXIT_EXCEPTION;
#define EXCEPTION_NOCRYPT(msg) EXIT_EXCEPTION
#endif

#ifdef DEBUG_BUILD
#define FATAL_EXCEPTION(msg) throw message_exception(std_crypt(msg), std::filesystem::path(std_crypt(__FILE__)).filename().string(), __LINE__)
#define FATAL_EXCEPTION_NOCRYPT(msg) throw message_exception(msg, std::filesystem::path(std_crypt(__FILE__)).filename().string(), __LINE__)
#endif

#ifndef DEBUG_BUILD
#define FATAL_EXCEPTION(msg) EXIT_EXCEPTION;
#define FATAL_EXCEPTION_NOCRYPT(msg) EXIT_EXCEPTION
#endif