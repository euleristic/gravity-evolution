// This file is an adequate gl wrapper for this project, it is not a general purpose solution.

#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#define GLM_SWIZZLE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GL/glext.h"

#include <memory>
#include <filesystem>
#include <fstream>
#include <concepts>

#ifdef _DEBUG
#include <iostream>
#define WRITE_TO_CERR(message) std::cerr << message
#else // !_DEBUG
#define WRITE_TO_CERR(message)
#endif // _DEBUG

namespace euleristic {
	namespace gl_wrapper {
		// Type aliases
		using scope_based_window = std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)>;

		// Calls glfwInit on construction and glfwTerminate() on destruction
		class library_lifetime {
		public:
			library_lifetime() {
				if (!glfwInit()) {
					WRITE_TO_CERR("Failed to initialize glfw\n");
					throw glfwGetError(nullptr);
				};
			}
			~library_lifetime() noexcept { glfwTerminate(); }
		};

		// GLFW window of scoped lifetime factory, throws if glfwCreateWindow returns nullptr
		template <std::floating_point component_type>
		static scope_based_window create_scoped_window(int width, int height, const char* const title) {

			// Set window hints
			if constexpr (std::same_as<component_type, float>) {
				// Version can be older
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			}
			else if constexpr (std::same_as<component_type, double>) {
				// Necessary GL version for double support
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
			}
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

			// Set window to not be resizable
			glfwWindowHint(GLFW_RESIZABLE, false);

			// RAII glfw window
			auto window = scope_based_window(glfwCreateWindow(width, height, title, nullptr, nullptr),
				[](GLFWwindow* window) { glfwDestroyWindow(window); });

			if (!window) {
				WRITE_TO_CERR("Failed to initialize glfw\n");
				throw glGetError();
			}

			return std::move(window);
		}

		class vertex_buffer {
			unsigned int id;

		public:
			vertex_buffer() noexcept {
				glGenBuffers(1, &id);
			}

			vertex_buffer(std::floating_point auto* buffer_data, size_t buffer_size) noexcept {
				glGenBuffers(1, &id);
				set(buffer_data, buffer_size);
			}

			void bind() noexcept {
				glBindBuffer(GL_ARRAY_BUFFER, id);
			}

			void set(std::floating_point auto* buffer_data, size_t buffer_size) noexcept {
				bind();
				glBufferData(GL_ARRAY_BUFFER, buffer_size * sizeof(buffer_data), buffer_data, GL_STATIC_DRAW);
			}
		};

		class vertex_shader {
			unsigned int id;
			friend class shader_program;
		public:
			vertex_shader() noexcept {
				id = glCreateShader(GL_VERTEX_SHADER);
			}

			vertex_shader(std::filesystem::path path) {
				id = glCreateShader(GL_VERTEX_SHADER);
				set(path);
			}

			~vertex_shader() noexcept {
				glDeleteShader(id);
			}

			void set(std::filesystem::path path) const {
				std::ifstream ifstream(path);
				if (!ifstream.is_open()) {
					WRITE_TO_CERR("Failed to open path: " << path << '\n');
					throw std::exception{};
				}
				std::stringstream stringstream;
				stringstream << ifstream.rdbuf();
				std::string string = stringstream.str();
				const char* source = string.c_str();
				glShaderSource(id, 1, &source, nullptr);
				glCompileShader(id);

				int success;
				glGetShaderiv(id, GL_COMPILE_STATUS, &success);
				if (!success) {
					char info_log[512];
					glGetShaderInfoLog(id, 512, nullptr, info_log);
					WRITE_TO_CERR("Could not compile vertex shader at path: " << path << "\nInfo log: " << info_log << '\n');
					throw glGetError();
				}

			}
		};

		class fragment_shader {
			unsigned int id;
			friend class shader_program;
		public:
			fragment_shader() noexcept {
				id = glCreateShader(GL_FRAGMENT_SHADER);
			}

			fragment_shader(std::filesystem::path path) {
				id = glCreateShader(GL_FRAGMENT_SHADER);
				set(path);
			}

			~fragment_shader() noexcept {
				glDeleteShader(id);
			}

			void set(std::filesystem::path path) const {
				std::ifstream ifstream(path);
				if (!ifstream.is_open()) {
					WRITE_TO_CERR("Failed to open path: " << path << '\n');
					throw std::exception{};
				}
				std::stringstream stringstream;
				stringstream << ifstream.rdbuf();
				std::string string = stringstream.str();
				const char* source = string.c_str();
				glShaderSource(id, 1, &source, nullptr);
				glCompileShader(id);

				int success;
				glGetShaderiv(id, GL_COMPILE_STATUS, &success);
				if (!success) {
					char info_log[512];
					glGetShaderInfoLog(id, 512, nullptr, info_log);
					WRITE_TO_CERR("Could not compile fragment shader at path: " << path << "\nInfo log: " << info_log << '\n');
					throw glGetError();
				}

			}
		};

		class shader_program {
			unsigned int id;
			template <typename T>
			friend class uniform;
		public:
			shader_program() noexcept {
				id = glCreateProgram();
			}

			shader_program(const vertex_shader& vs, const fragment_shader& fs) {
				id = glCreateProgram();
				set(vs, fs);
				use();
			}

			~shader_program() {
				glDeleteProgram(id);
			}

			void set(const vertex_shader& vs, const fragment_shader& fs) const {
				glAttachShader(id, vs.id);
				glAttachShader(id, fs.id);
				glLinkProgram(id);

				int success;
				glGetProgramiv(id, GL_LINK_STATUS, &success);
				if (!success) {
					char info_log[512];
					glGetProgramInfoLog(id, 512, nullptr, info_log);
					WRITE_TO_CERR("Failed to link shaders.\n");
					throw glGetError();
				}
			}
			
			void use() const {
				glUseProgram(id);
			}
		};

		template <typename uniform_type>
		class uniform {
			unsigned int location;
			uniform() = delete;
		public:
			uniform(const shader_program& shader, const char* identifier) noexcept {
				location = glGetUniformLocation(shader.id, identifier);
			}

			void set(const uniform_type& value) noexcept {
				if constexpr (std::same_as<uniform_type, glm::mat4>) {
					glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
				}
				else if constexpr (std::same_as<uniform_type, glm::dmat4>) {
					glUniformMatrix4dv(location, 1, GL_FALSE, glm::value_ptr(value));
				}
			}
		};

		class vertex_array {
			unsigned int id;
		public:
			vertex_array() noexcept {
				glGenVertexArrays(1, &id);
			}

			void bind() noexcept {
				glBindVertexArray(id);
			}

			void set_attrib_pointer(GLuint index, GLint size, GLenum type, GLsizei stride, size_t offset) noexcept {
				bind();
				glVertexAttribPointer(index, size, type, false, stride, (void*)offset);
				glEnableVertexAttribArray(0);
			}

			void render(GLenum mode, GLint first, GLsizei count) {
				bind();
				glDrawArrays(mode, first, count);
			}
		};
	};
}