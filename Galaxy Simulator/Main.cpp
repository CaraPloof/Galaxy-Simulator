#include "galaxy.h"

class Galaxy: public app {
protected:

	VertexArray va;
	Buffer ssbo;
	Renderer renderer;
	Shader shader;
	Shader Cshader;
	std::vector<glm::vec4> vertices;
	float speedHack;
	unsigned int number = 1000000;
	float accuracy = 0.0001f;
	float iniVel = 0.07f;
public:
	void init() override {

		for (float i = 0; i < number; i++) {
			float angle = (float)(rand() % 62831852)/1000;
			float len = (float)(rand() % 1000)/1000;
			vertices.push_back(glm::vec4(cos(angle)*len, 1*(float)(rand() % 100) / 1000, sin(angle) * len,0));
			vertices.push_back(glm::vec4(-sin(angle), 0, cos(angle), 0) * iniVel);
			vertices.push_back(glm::vec4(0, 0, 0, 0)- glm::vec4(cos(angle) * len*100,0, sin(angle) * len*100, 0)*1000000.f);
		}

		va.createVertexArray();
		ssbo.CreateBuffer(sizeof(glm::vec4)*vertices.size(), vertices, GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_COPY);

		shader.createVFShader("core/shaders/Basic.vert", "core/shaders/Basic.frag");
		Cshader.createCShader("core/shaders/compute.glsl");

		va.Unbind();
		ssbo.Unbind(GL_SHADER_STORAGE_BUFFER);
		shader.Unind();	
	}

	void mainLoop(float& dt) override {
		speedHack = 1.f;;
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			speedHack = 100.f;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cameraPos += glm::normalize(glm::vec3(1.0, 0.0, 1.0) * cameraFront) * dt * speedHack;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cameraPos -= glm::normalize(glm::vec3(1.0, 0.0, 1.0) * cameraFront) * dt * speedHack;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * dt * speedHack;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * dt * speedHack;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			cameraPos += cameraUp * dt * speedHack;
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			cameraPos -= cameraUp * dt * speedHack;
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 projection = glm::perspective(glm::radians(110.0f), (float)width / (float)height, 0.0001f, 20000.0f);

		ImGui::Begin("test");
		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		Cshader.useCompute(ssbo, floor(vertices.size() / 128.f));
		Cshader.setFloat("dt", dt);
		Cshader.setFloat("accuracy", accuracy);
		va.AddBuffer(ssbo,0, GL_ARRAY_BUFFER, 3, 12, 0);
		shader.Bind();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		renderer.DrawArray(va, shader, number);
	}
};

int main(void){
	std::unique_ptr<Galaxy> galaxy = std::make_unique<Galaxy>();
	galaxy->start();
}