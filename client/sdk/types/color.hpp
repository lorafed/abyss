#pragma once
#include <sstream>

#define IMGUI_DEFINE_MATH_OPERATORS

#include <ext/imgui/imgui.h>
#include <algorithm>

struct color
{
    float r, g, b, a;

    int hex_to_decimal(const std::string& hex_color) const
    {
        std::stringstream string_stream;
        string_stream << std::hex << hex_color;
        int decimal;
        string_stream >> decimal;

        return decimal;
    }

    color(int red, int green, int blue, int alpha = 255) {
        r = static_cast<float>(red); g = static_cast<float>(green); b = static_cast<float>(blue); a = static_cast<float>(alpha);
    }

    color(float red, float green, float blue, float alpha = 255) {
        r = red; g = green; b = blue; a = alpha;
    }

    color(const char* hex)
    {
        const auto hex_string = std::string(hex);

        r = static_cast<float>(hex_to_decimal(hex_string.substr(0, 2)));
        g = static_cast<float>(hex_to_decimal(hex_string.substr(2, 2)));
        b = static_cast<float>(hex_to_decimal(hex_string.substr(4, 2)));
        a = 255.F;
    }

    color(const std::string& hex)
    {
        r = static_cast<float>(hex_to_decimal(hex.substr(0, 2)));
        g = static_cast<float>(hex_to_decimal(hex.substr(2, 2)));
        b = static_cast<float>(hex_to_decimal(hex.substr(4, 2)));
        a = 255.F;
    }

    color(const ImVec4 color)
    {
        r = color.x * 255.F;
        g = color.y * 255.F;
        b = color.z * 255.F;
        a = color.w * 255.F;
    }

    color(const ImColor color)
    {
        r = color.Value.x * 255.F;
        g = color.Value.y * 255.F;
        b = color.Value.z * 255.F;
        a = color.Value.w * 255.F;
    }

    ImVec4 get_color_vec4() const {
        return { r / 255.F , g / 255.F, b / 255.F, a / 255.F };
    }

    ImU32 get_color_u32() const {
        return ImGui::GetColorU32(get_color_vec4());
    }

    ImColor get_im_color() const {
        return IM_COL32(r, g, b, a);
    }

    operator ImVec4() const {
        return { r / 255.F , g / 255.F, b / 255.F, a / 255.F };
    }

    operator ImU32() const {
        return ImGui::GetColorU32(get_color_vec4());
    }

    color darken(float factor)
    {
        factor = std::clamp(factor, 0.01F, 1.F);

        r *= (1.0f - factor);
        g *= (1.0f - factor);
        b *= (1.0f - factor);

        return color(r, g, b, a);
    }

    color set_alpha(float alpha)
    {
        a = alpha;
        return color(r, g, b, a);
    }

    friend std::ostream& operator<<(std::ostream& output_stream, const color& color)
    {
        output_stream << "ColorU: {R: " << color.r << ", G: " << color.g << ", B: " << color.b << ", A: " << color.a << "}";
        return output_stream;
    }
};
