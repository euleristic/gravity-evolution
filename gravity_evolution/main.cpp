#include "gl_wrapper.hpp"
#include "gravitational_system.hpp"

using namespace euleristic;

int main() {

	// Initialize glfw
	gl_wrapper::library_lifetime gl_life;
	
	auto window = gl_wrapper::create_scoped_window(width, height, "Gravity Evolution");

	glfwMakeContextCurrent(window.get());

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		WRITE_TO_CERR("Failed to initialize GLAD\n");
		return -1;
	}

	glViewport(0, 0, width, height);

	gl_wrapper::shader_program shader({ "vertex_shader.glsl" }, { "fragment_shader.glsl" });

	gl_wrapper::vertex_array vao;

	gravitational_system galaxy;

	vao.bind();
	gl_wrapper::vertex_buffer vbo(galaxy.vertices(), star_count * 3);
	vao.set_attrib_pointer(0, 3, GL_FLOAT, 3 * sizeof(float), 0);

	auto projection = glm::perspective(45.0f, static_cast<float>(width) / static_cast<float>(height), 1.0f, 100.0f);
	gl_wrapper::uniform<glm::mat4> projection_uniform(shader, "projection");
	auto model = glm::mat4(1.0f);

	auto last_frame = std::chrono::steady_clock::now();
	while (!glfwWindowShouldClose(window.get())) {
		
		// Handle input
		if (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window.get(), true);
		if (glfwGetKey(window.get(), GLFW_KEY_W) == GLFW_PRESS) 
			model = glm::rotate(glm::mat4(1.0f), pitch_speed * delta_time.count(), right) * model;
		if (glfwGetKey(window.get(), GLFW_KEY_S) == GLFW_PRESS)
			model = glm::rotate(glm::mat4(1.0f), pitch_speed * delta_time.count(), left) * model;
		if (glfwGetKey(window.get(), GLFW_KEY_A) == GLFW_PRESS)
			model = glm::rotate(glm::mat4(1.0f), yaw_speed * delta_time.count(), up) * model;
		if (glfwGetKey(window.get(), GLFW_KEY_D) == GLFW_PRESS)
			model = glm::rotate(glm::mat4(1.0f), yaw_speed * delta_time.count(), down) * model;
		if (glfwGetKey(window.get(), GLFW_KEY_Q) == GLFW_PRESS)
			model = glm::rotate(glm::mat4(1.0f), roll_speed * delta_time.count(), forward) * model;
		if (glfwGetKey(window.get(), GLFW_KEY_E) == GLFW_PRESS)
			model = glm::rotate(glm::mat4(1.0f), roll_speed * delta_time.count(), backward) * model;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		vbo.set(galaxy.vertices(), star_count * 3);
		projection_uniform.set(projection * view * model);
		shader.use();
		vao.render(GL_POINTS, 0, star_count);
		
		glfwSwapBuffers(window.get());
		glfwPollEvents();

		galaxy.update_positions();
		galaxy.update_velocities_chunk(0, star_count);
		galaxy.step();

		//std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_frame) << '\n';
		while (std::chrono::steady_clock::now() - last_frame < delta_time); //wait
		last_frame = std::chrono::steady_clock::now();
	};
	
	return 0;
}