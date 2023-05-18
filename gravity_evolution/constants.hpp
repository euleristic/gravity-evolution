// This file contains the definitions for all constants used in the program
// See this as a config file, feel free to mess around and vary the independent values!

#pragma once
#include <chrono>
#include <cmath>
#include "gl_wrapper.hpp"
#include "glm/glm.hpp"

namespace euleristic {

	using component_type = float; // float or double supported

	using vec3_t = glm::vec<3, component_type>;
	using vec4_t = glm::vec<4, component_type>;
	using mat4_t = glm::mat<4, 4, component_type>;

	constexpr size_t star_count = 2048;
	constexpr component_type spawn_range = 30.0;
	constexpr component_type velocity_range = 3.0;

	constexpr int width = 1024;
	constexpr int height = 1024;

	using namespace std::chrono_literals;
	constexpr std::chrono::duration<component_type> delta_time = 20.0ms;
	constexpr component_type gravitational_constant = 0.10;

	constexpr size_t additional_thread_count = 4;

	constexpr mat4_t camera_translation(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, -100.0, 1.0);
	constexpr vec3_t right(1.0, 0.0, 0.0);
	constexpr vec3_t left(-1.0, 0.0, 0.0);
	constexpr vec3_t up(0.0, 1.0, 0.0);
	constexpr vec3_t down(0.0, -1.0, 0.0);
	constexpr vec3_t forward(0.0, 0.0, -1.0);
	constexpr vec3_t backward(0.0, 0.0, 1.0);
	constexpr component_type rotation_speed = 1.0;
	constexpr component_type translation_speed = 10.0;

	// Don't touch:

	constexpr size_t chunk_size = star_count / additional_thread_count;
	constexpr size_t remainder_index = additional_thread_count * chunk_size;

	constexpr GLenum gl_type_rep() noexcept {
		if constexpr (std::same_as<component_type, float>) return GL_FLOAT;
		else if constexpr (std::same_as < component_type, double>) return GL_DOUBLE;
	}

	constexpr component_type delta_g = delta_time.count() * gravitational_constant;
	constexpr component_type delta_rotation = delta_time.count() * rotation_speed;
	constexpr component_type delta_translation = delta_time.count() * translation_speed;

}