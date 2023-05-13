// This file defines the behavior of the gravitational system

#pragma once
#include "constants.hpp"
#include "glm/gtx/norm.hpp"
#include <array>
#include <random>
#include <numeric>

namespace euleristic {
	class gravitational_system {
		std::array<float, star_count * 3> previous_positions, next_positions, previous_velocities{}, next_velocities;

		const glm::vec3 vector_at(const float* arr, size_t index) const {
			return glm::vec3(arr[index * 3], arr[index * 3 + 1], arr[index * 3 + 2]);
		}

	public:

		const float* vertices() const {
			return previous_positions.data();
		}

		gravitational_system() {
			// Randomize
			std::random_device device;
			std::mt19937 engine(device());
			std::uniform_real_distribution<float> normalized_uniform_distribution; // In the sense 0 < n < 1
			for (size_t i = 0; i < star_count; ++i) {

				// Credit for the uniform sphere distribution algorithm used here to Cory Simon: http://corysimon.github.io/articles/uniformdistn-on-sphere/
				// The square root of the normalized uniform distribution then uniformly distributes in the ball

				float scale = sqrtf(normalized_uniform_distribution(engine)) * spawn_range;
				float theta = 2.0f * glm::pi<float>() * normalized_uniform_distribution(engine);
				float phi = acos(1.0f - 2.0f * normalized_uniform_distribution(engine));
				previous_positions[i * 3] = sin(phi) * cos(theta) * scale;
				previous_positions[i * 3 + 1] = sin(phi) * sin(theta) * scale;
				previous_positions[i * 3 + 2] = cos(phi) * scale;

				scale = sqrtf(normalized_uniform_distribution(engine)) * velocity_range;
				theta = 2.0f * glm::pi<float>() * normalized_uniform_distribution(engine);
				phi = acos(1.0f - 2.0f * normalized_uniform_distribution(engine));
				previous_velocities[i * 3] = sin(phi) * cos(theta) * scale;
				previous_velocities[i * 3 + 1] = sin(phi) * sin(theta) * scale;
				previous_velocities[i * 3 + 2] = cos(phi) * scale;
			}

		}

		// begin and end should be the indices of the vector
		void update_velocities_chunk(const size_t begin, const size_t end) {
			for (size_t i = begin; i < end; ++i) {
				auto position = vector_at(previous_positions.data(), i);
				auto velocity = vector_at(previous_velocities.data(), i);
				for (size_t j = 0; j < star_count; ++j) {
					if (i == j) {
						continue;
					}
					glm::vec3 relative = vector_at(previous_positions.data(), j) - position;
					float magnitute_sqr = glm::length2(relative);

					// Masses being unit gives, according to Newton:
					velocity += relative * gravitational_constant * delta_time.count() / (magnitute_sqr * sqrtf(magnitute_sqr));
				}
				velocity = glm::length2(velocity) < max_velocity_sqr ? velocity : glm::normalize(velocity) * max_velocity;

				next_velocities[i * 3] = velocity.x;
				next_velocities[i * 3 + 1] = velocity.y;
				next_velocities[i * 3 + 2] = velocity.z;
			}
		}

		void update_positions() {
			for (size_t i = 0; i < star_count * 3; ++i) {
				next_positions[i] = previous_positions[i] + previous_velocities[i] * delta_time.count();
			}
		}

		void step() {
			std::swap(previous_positions, next_positions);
			std::swap(previous_velocities, next_velocities);
		}
	};
}