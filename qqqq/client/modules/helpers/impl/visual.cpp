#include "pch.hpp"
#include "../helpers.hpp"

#include "gl/GL.h"

#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.hpp"

unsigned long long helpers::render::load_texture(void* buffer, int size)
{
	int image_width = 0;
	int image_height = 0;

	unsigned char* image_data = stbi_load_from_memory((unsigned char*)buffer, size, &image_width, &image_height, nullptr, 4);

	if (image_data == nullptr)
	{
		return 0;
	}

	unsigned int texture_id = 0;

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexParameteri(GL_TEXTURE_2D, 0x2802, 0x2901); // GL_TEXTURE_WRAP_S, GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, 0x2803, 0x2901); // GL_TEXTURE_WRAP_T, GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, 0x2801, 0x2601); // GL_TEXTURE_MIN_FILTER, GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, 0x2800, 0x2601); // GL_TEXTURE_MAG_FILTER, GL_LINEAR

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

	stbi_image_free(image_data);

	return static_cast<unsigned long long>(texture_id);
}

bool helpers::render::world2screen(const matrix& projection, const matrix& model_view, const vec2<float>& screen_size, const vec3<double>& position, vec2<float>& screen_position)
{
	vec4<float> csp = vec4<float>(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z), 1.f);
	csp = (csp.multiply(model_view.to_vector())).multiply(projection.to_vector());

	vec3<float> ndc = vec3<float>(csp.x / csp.w, csp.y / csp.w, csp.z / csp.w);

	if (ndc.z > 1.f && ndc.z < 1.15f)
	{
		screen_position = vec2(((ndc.x + 1.f) / 2.f) * screen_size.x, ((1.f - ndc.y) / 2.f) * screen_size.y);
		return true;
	}

	return false;
}

matrix helpers::render::calculate_projection(const vec2<float>& screen_size, float fov)
{
	matrix projection = { };

	float fov_corrected = tan(fov * 0.0087266462599716f);
	float aspect_ratio = screen_size.x / screen_size.y;

	projection.m00 = 1.f / (aspect_ratio * fov_corrected);
	projection.m01 = 0.f;
	projection.m02 = 0.f;
	projection.m03 = 0.f;
	projection.m10 = 0.f;
	projection.m11 = 1.f / fov_corrected;
	projection.m12 = 0.f;
	projection.m13 = 0.f;
	projection.m20 = 0.f;
	projection.m21 = 0.f;
	projection.m22 = -1.00058f;
	projection.m23 = -1.f;
	projection.m30 = 0.f;
	projection.m31 = 0.f;
	projection.m32 = -0.100029f;
	projection.m33 = 0.f;

	return projection;
}

matrix helpers::render::caclulate_modelview(const vec2<float>& angles)
{
	matrix modelview = { };

	vec2 view_angles =
	{
		angles.x * 0.0174532925199433f,
		angles.y * 0.0174532925199433f
	};

	float rotation_x = cos(view_angles.y);
	float rotation_y = sin(view_angles.x);
	float rotation_z = sin(view_angles.y);
	float rotation_yz = -rotation_z * sin(view_angles.x);
	float rotation_xz = cos(view_angles.x);
	float rotation_yx = rotation_x * sin(view_angles.x);

	modelview.m00 = -rotation_x;
	modelview.m01 = rotation_yz;
	modelview.m02 = (cos(view_angles.x) * sin(view_angles.y));
	modelview.m03 = 0.f;
	modelview.m10 = 0.f;
	modelview.m11 = rotation_xz;
	modelview.m12 = rotation_y;
	modelview.m13 = 0.f;
	modelview.m20 = -rotation_z;
	modelview.m21 = rotation_yx;
	modelview.m22 = (-cos(view_angles.x) * cos(view_angles.y));
	modelview.m23 = 0.f;
	modelview.m30 = 0.f;
	modelview.m31 = -(cos(abs(view_angles.x)) * 1.62f);
	modelview.m32 = get_eye_vertical_point(static_cast<int>(view_angles.x / 0.0174532925199433));
	modelview.m33 = 1.f;

	return modelview;
}
