// This file contains the definitions for all constants used in the program

#pragma once
#include <chrono>
#include "glm/glm.hpp"

namespace euleristic {
	constexpr size_t star_count = 1024;
	constexpr float spawn_range = 2.0f;
	constexpr float velocity_range = 5.0f;
	constexpr int spawn_precision = 5000;
	constexpr float max_velocity = 10.f;

	constexpr float conversion_factor = spawn_range / static_cast<float>(spawn_precision);
	constexpr float max_velocity_sqr = max_velocity * max_velocity;

	constexpr int width = 1024;
	constexpr int height = 1024;

	using namespace std::chrono_literals;
	constexpr std::chrono::duration<float> delta_time = 20.0ms;
	constexpr float gravitational_constant = 0.01f;

	constexpr glm::mat4 view(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, -5.0, 1.0);
	constexpr glm::vec3 right(1.0f, 0.0f, 0.0f);
	constexpr glm::vec3 left(-1.0f, 0.0f, 0.0f);
	constexpr glm::vec3 up(0.0f, 1.0f, 0.0f);
	constexpr glm::vec3 down(0.0f, -1.0f, 0.0f);
	constexpr glm::vec3 forward(0.0f, 0.0f, 1.0f);
	constexpr glm::vec3 backward(0.0f, 0.0f, -1.0f);
	constexpr float yaw_speed = 1.0f;
	constexpr float pitch_speed = 1.0f;
	constexpr float roll_speed = 1.0f;
}