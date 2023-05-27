#include "gravitational_system.hpp"
#include <array>

using namespace euleristic;
using namespace std::string_literals;

int main() {

	try {

		// Initialize glfw
		gl_wrapper::library_lifetime gl_life;

		auto window = gl_wrapper::create_scoped_window<component_type>(width, height, "Gravity Evolution");

		glfwMakeContextCurrent(window.get());

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			WRITE_TO_CERR("Failed to initialize GLAD\n");
			return -1;
		}

		glViewport(0, 0, width, height);

		gl_wrapper::shader_program shader;
		if (std::same_as<component_type, double>) {
			glEnable(GL_ARB_gpu_shader_fp64);
			shader = gl_wrapper::shader_program({ "vertex_shader_double.glsl" }, { "fragment_shader.glsl" });
		}
		else
			shader = gl_wrapper::shader_program({ "vertex_shader_float.glsl" }, { "fragment_shader.glsl" });

		gl_wrapper::vertex_array vao;

		gravitational_system galaxy;
		std::array<std::unique_ptr<acceleration_thread>, additional_thread_count> threads;
		for (size_t i = 0; i < threads.size(); ++i) {
			threads[i] = std::make_unique<acceleration_thread>(i * chunk_size, (i + 1) * chunk_size, galaxy);
		}



		vao.bind();
		gl_wrapper::vertex_buffer vbo(galaxy.vertices(), star_count * 3);
		vao.set_attrib_pointer(0, 3, gl_type_rep(), 3 * sizeof(component_type), 0);

		auto projection = glm::perspective<component_type>(45.0, static_cast<component_type>(width) / static_cast<component_type>(height), 1.0, 500.0);
		gl_wrapper::uniform<mat4_t> projection_uniform(shader, "projection");
		auto camera_rotation = mat4_t(1.0);
		auto model_translation = mat4_t(1.0);

		auto last_frame = std::chrono::steady_clock::now();
		while (!glfwWindowShouldClose(window.get())) {
			// Immediately start threads
			for (auto& thread : threads) {
				thread->begin();
			}

			// Handle input
			if (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window.get(), true);
			if (glfwGetKey(window.get(), GLFW_KEY_W) == GLFW_PRESS)
				camera_rotation = glm::rotate(mat4_t(1.0f), delta_rotation, right) * camera_rotation;
			if (glfwGetKey(window.get(), GLFW_KEY_S) == GLFW_PRESS)
				camera_rotation = glm::rotate(mat4_t(1.0f), delta_rotation, left) * camera_rotation;
			if (glfwGetKey(window.get(), GLFW_KEY_A) == GLFW_PRESS)
				camera_rotation = glm::rotate(mat4_t(1.0f), delta_rotation, up) * camera_rotation;
			if (glfwGetKey(window.get(), GLFW_KEY_D) == GLFW_PRESS)
				camera_rotation = glm::rotate(mat4_t(1.0f), delta_rotation, down) * camera_rotation;
			if (glfwGetKey(window.get(), GLFW_KEY_Q) == GLFW_PRESS)
				camera_rotation = glm::rotate(mat4_t(1.0f), delta_rotation, backward) * camera_rotation;
			if (glfwGetKey(window.get(), GLFW_KEY_E) == GLFW_PRESS)
				camera_rotation = glm::rotate(mat4_t(1.0f), delta_rotation, forward) * camera_rotation;

			if (glfwGetKey(window.get(), GLFW_KEY_UP) == GLFW_PRESS)
				model_translation = glm::translate(model_translation, delta_translation * (glm::inverse(camera_rotation) * vec4_t(backward, 1.0)).xyz);
			if (glfwGetKey(window.get(), GLFW_KEY_DOWN) == GLFW_PRESS)
				model_translation = glm::translate(model_translation, delta_translation * (glm::inverse(camera_rotation) * vec4_t(forward, 1.0)).xyz);
			if (glfwGetKey(window.get(), GLFW_KEY_LEFT) == GLFW_PRESS)
				model_translation = glm::translate(model_translation, delta_translation * (glm::inverse(camera_rotation) * vec4_t(right, 1.0)).xyz);
			if (glfwGetKey(window.get(), GLFW_KEY_RIGHT) == GLFW_PRESS)
				model_translation = glm::translate(model_translation, delta_translation * (glm::inverse(camera_rotation) * vec4_t(left, 1.0)).xyz);
			if (glfwGetKey(window.get(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
				model_translation = glm::translate(model_translation, delta_translation * (glm::inverse(camera_rotation) * vec4_t(down, 1.0)).xyz);
			if (glfwGetKey(window.get(), GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
				model_translation = glm::translate(model_translation, delta_translation * (glm::inverse(camera_rotation) * vec4_t(up, 1.0)).xyz);


			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			vbo.set(galaxy.vertices(), star_count * 3);
			projection_uniform.set(projection * camera_translation * camera_rotation * model_translation);
			shader.use();
			vao.render(GL_POINTS, 0, star_count);

			glfwSwapBuffers(window.get());
			glfwPollEvents();

			galaxy.update_positions();
			galaxy.update_velocities_chunk(remainder_index, star_count);

			for (auto& thread : threads) {
				thread->await();
			}

			galaxy.step();

			glfwSetWindowTitle(window.get(), ("Gravity Evolution. Frame busy time: "s +
				std::to_string(std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(std::chrono::steady_clock::now() - last_frame).count()) + "ms").c_str());
			while (std::chrono::steady_clock::now() - last_frame < delta_time); //wait
			last_frame = std::chrono::steady_clock::now();
		};
	} catch (GLenum error_code) {
		return (int)error_code;
	}
	
	return 0;
}