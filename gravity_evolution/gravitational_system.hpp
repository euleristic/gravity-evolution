// This file defines the behavior of the gravitational system

#pragma once
#include "constants.hpp"
#include "glm/gtx/norm.hpp"
#include <random>
#include <memory>
#include <semaphore>

namespace euleristic {
	class gravitational_system {
		std::unique_ptr<component_type[]> previous_positions, next_positions, previous_velocities, next_velocities;

		const vec3_t vector_at(const component_type* arr, size_t index) const {
			return vec3_t(arr[index * 3], arr[index * 3 + 1], arr[index * 3 + 2]);
		}

	public:

		const component_type* vertices() const {
			return previous_positions.get();
		}

		gravitational_system() {

			previous_positions  = std::make_unique<component_type[]>(star_count * 3);
			next_positions      = std::make_unique<component_type[]>(star_count * 3);
			previous_velocities = std::make_unique<component_type[]>(star_count * 3);
			next_velocities     = std::make_unique<component_type[]>(star_count * 3);

			// Randomize starting positions and velocities
			std::random_device device;
			std::mt19937 engine(device());
			std::uniform_real_distribution<component_type> normalized_uniform_distribution; // In the sense 0 < n < 1
			for (size_t i = 0; i < star_count; ++i) {

				// Credit for the uniform sphere distribution algorithm used here to Cory Simon: http://corysimon.github.io/articles/uniformdistn-on-sphere/
				// The square root of the normalized uniform distribution then uniformly distributes in the ball

				component_type scale = sqrt(normalized_uniform_distribution(engine)) * spawn_range;;
				component_type theta = static_cast<component_type>(2.0) * glm::pi<component_type>() * normalized_uniform_distribution(engine);
				component_type phi = acos(static_cast<component_type>(1.0) - static_cast<component_type>(2.0) * normalized_uniform_distribution(engine));
				previous_positions[i * 3] = sin(phi) * cos(theta) * scale;
				previous_positions[i * 3 + 1] = sin(phi) * sin(theta) * scale;
				previous_positions[i * 3 + 2] = cos(phi) * scale;

				scale = sqrt(normalized_uniform_distribution(engine)) * velocity_range;
				theta = static_cast<component_type>(2.0) * glm::pi<component_type>() * normalized_uniform_distribution(engine);
				phi = acos(static_cast<component_type>(1.0) - static_cast<component_type>(2.0) * normalized_uniform_distribution(engine));
				previous_velocities[i * 3] = sin(phi) * cos(theta) * scale;
				previous_velocities[i * 3 + 1] = sin(phi) * sin(theta) * scale;
				previous_velocities[i * 3 + 2] = cos(phi) * scale;
			}
		}



		// begin and end should be the indices of the vec3s (so lower than star_count)
		void update_velocities_chunk(const size_t begin, const size_t end) {
			for (size_t i = begin; i < end; ++i) {
				auto position = vector_at(previous_positions.get(), i);
				auto velocity = vector_at(previous_velocities.get(), i);
				for (size_t j = 0; j < star_count; ++j) {

					if (i == j) {
						continue;
					}
					vec3_t relative = vector_at(previous_positions.get(), j) - position;
					auto magnitude_sqr = glm::length2(relative);

					auto scale = delta_g / magnitude_sqr;

					// Masses being unit gives, according to Newton:
					velocity += scale * relative / sqrt(magnitude_sqr);
				}

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

	// A thread which updates a chunk 
	class acceleration_thread {
		std::binary_semaphore begin_signal{ 0 }, complete_signal{ 0 };
		bool dead{};
		size_t from, to;
		std::thread thread;

	public:
		acceleration_thread() = delete;
		acceleration_thread(size_t from, size_t to, gravitational_system& galaxy) noexcept : from(from), to(to) {
			thread = std::thread([&]() {
				while (true) {
					begin_signal.acquire(); // Wait to begin
					if (dead) // Destructor has been called
						break;
					galaxy.update_velocities_chunk(this->from, this->to);
					complete_signal.release(); // Signal that calculation operation is complete
				}
			});
		}

		~acceleration_thread() noexcept {
			dead = true; // No race condition, begin_signal is guarding the reading of dead
			begin_signal.release();
			thread.join();
		}

		void begin() noexcept {
			begin_signal.release();
		}

		void await() noexcept {
			complete_signal.acquire();
		}
	};
}