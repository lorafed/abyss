#include "pch.hpp"
#include "sdk.hpp"

bool string::compare(const std::string& str1, const std::string& str2)
{
    return (str1 == str2);
}

std::string string::to_lowercase(const std::string& input)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string string::to_uppercase(const std::string& input)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string string::trim(const std::string& input)
{
    auto start = input.begin();
    while (start != input.end() && std::isspace(*start)) {
        ++start;
    }

    auto end = input.end();
    do {
        --end;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}

bool string::contains(const std::string& str, const std::string& substring)
{
    return str.find(substring) != std::string::npos;
}

std::vector<std::string> string::split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(str);
    while (std::getline(token_stream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string string::join(const std::vector<std::string>& elements, const std::string& delimiter)
{
    std::ostringstream os;
    for (size_t i = 0; i < elements.size(); ++i) {
        os << elements[i];
        if (i != elements.size() - 1) {
            os << delimiter;
        }
    }
    return os.str();
}
