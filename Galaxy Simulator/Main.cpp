#include "galaxy.h"

class Galaxy: public app {
protected:
	VertexArray va;
	Buffer ssbo;
	Renderer renderer;
	Shader shader;
	Shader Cshader;
	std::vector<glm::vec4> vertices;
	glm::mat4 view;
	glm::mat4 projection;
	char preset[15] = "default";
	char presetList[100];

	int number = 100000;
	float accuracy = 0.01f;
	float scale = 1;

	float gravity = 1e-5;
	float csmooth = 1e-1;
	float iniVel = 0.5f;
	float blackHole = 1;

	glm::vec3 hue = glm::vec3(1, 1, 1);

	float speedHack = 1.f;

	bool fullscrean = true;
	bool hold = false;
public:
	void init() override {
		// appel�e au lancement du programme
		reinit();

		va.createVertexArray();
		ssbo.CreateBuffer(sizeof(glm::vec4)*vertices.size(), vertices, GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_COPY);

		shader.createVFShader("core/shaders/Basic.vert", "core/shaders/Basic.frag");
		Cshader.createCShader("core/shaders/compute.glsl");

		va.Unbind();
		ssbo.Unbind(GL_SHADER_STORAGE_BUFFER);
		shader.Unind();	

		projection = glm::perspective(glm::radians(110.0f), (float)width / (float)height, 0.0001f, 20000.0f);
	}

	void reinit() {
		// reinitialise la simulation
		updatePresetList();
		vertices.clear();
		for (float i = 0; i < number; i++) {
			float angle = (float)(rand() % 62831852) / 1000;
			float len = scale * (float)(rand() % 1000) / 1000;
			vertices.push_back(glm::vec4(cos(angle) * len, 1 * (float)(rand() % 100) / 1000, sin(angle) * len, 0));
			vertices.push_back(glm::vec4(-sin(angle), 0, cos(angle), 0) * iniVel);
			vertices.push_back(glm::vec4(0, 0, 0, 0) - glm::vec4(cos(angle) * len * 100, 0, sin(angle) * len * 100, 0));
		}
	}

	void savePresets(std::string name) {
		// sauvegarde le pr�reglage "name"
		std::ofstream file("presets.gpf", std::ios::app);
		if (file.is_open())
		{
			std::ifstream names("presets.gpf", std::ios::app);
			std::string line;
			while (getline(names, line))
			{
				if (line == name) {
					names.close();
					return;
				}
			}
			file << name << "\n";
			file << number << " " << accuracy << " " << scale << " " << gravity << " " << csmooth << " " << iniVel << " " << blackHole << " " << hue[0] << " " << hue[1] << " " << hue[2] << " " << speedHack <<"\n";
			file.close();
		}
		else std::cout << "Unable to open file";
		updatePresetList();
	}

	void loadPresets(std::string name) {
		// charge la pr�reglage "name"
		std::string line;
		std::ifstream file("presets.gpf", std::ios_base::in);
		int _nbre;
		float _preci;
		float _taille;

		float _gravit;
		float _smooth;
		float _velocite;
		float _tNoir;

		glm::vec3 _col;
		
		float _speed;

		if (file.is_open())
		{
			while (getline(file, line))
			{
				if (line == name) {
					std::string values;
					std::getline(file, values);
					std::stringstream sValues;
					sValues << values;
					sValues >> _nbre >> _preci >> _taille >> _gravit >> _smooth >> _velocite >> _tNoir >> _col[0] >> _col[1] >> _col[2] >> _speed;

					number = _nbre;
					accuracy = _preci;
					scale = _taille;

					gravity = _gravit;
					csmooth = _smooth;
					iniVel = _velocite;
					blackHole = _tNoir;

					hue = _col;

					speedHack = _speed;
					init();
					break;
				}
			}
			file.close();
		}
		else std::cout << "Unable to open file";
		updatePresetList();
	}

	void deletePreset(std::string name) {
		// supprime le pr�reglage "name"
		std::string line;
		std::ifstream fin;

		fin.open("presets.gpf");
		std::ofstream temp;
		temp.open("temp.txt");

		while (getline(fin, line)) {
			if (line != name)
				temp << line << std::endl;
			else getline(fin, line);
		}
		temp.close();
		fin.close();
		remove("presets.gpf");
		rename("temp.txt", "presets.gpf");
		updatePresetList();
	}

	void updatePresetList() {
		// met � jour la liste des pr�reglages dans l'interface graphique
		std::string list;
		std::string line;
		std::ifstream file("presets.gpf", std::ios_base::in);
		while (getline(file, line)) {
			list += line + ", ";
			getline(file, line);
		}
		strcpy_s(presetList, list.c_str());
	}

	void GUI(float& dt) {
		// dessine l'interface graphique avec ses fonctions
		ImGui::Text("simulation");
		ImGui::Text(("fps : "+std::to_string(1 / dt)).c_str());
		if (ImGui::Button("Restart")) { init(); }
		ImGui::InputText("nom du prereglage", preset, IM_ARRAYSIZE(preset));
		ImGui::Text("prereglages enregistres :");
		ImGui::Text(presetList);
		if (ImGui::Button("enregistrer")) { savePresets(preset); }
		if (ImGui::Button("charger")) { loadPresets(preset); }
		if (ImGui::Button("supprimer")) { deletePreset(preset); }
		ImGui::Text("emetteur");
		ImGui::SliderInt("nombre de particle", &number, 128, 5000000);
		ImGui::InputInt("nombre manuelle", &number);
		ImGui::InputFloat("precision", &accuracy, 0.00001, NULL, "%.5f");
		ImGui::SliderFloat("taille", &scale, 0, 100);
		ImGui::Text("physique");
		ImGui::InputFloat("gravite", &gravity, 0.00000001f, NULL, "%.8f");
		ImGui::InputFloat("csmooth", &csmooth, 0.0000001f, NULL, "%.8f");
		ImGui::InputFloat("velocite", &iniVel, 0.001f, NULL, "%.4f");
		ImGui::SliderFloat("troup noir", &blackHole, 0, 100);
		ImGui::Text("apparence");
		ImGui::SliderFloat3("hue", &hue[0], -1, 1);
		ImGui::Text("utilisateur");
		ImGui::SliderFloat("speedHack", &speedHack, 0, 1000);
		if (ImGui::Button("tp : x0 y0 z0")) { cameraPos = glm::vec3(0,0,0); }
	}

	void inputs(float& dt) {
		// interraction avec le clavier
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
		if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
			if (!hold) {
				std::cout << "lol";
				if (fullscrean) {
					app::width = 1280;
					app::height = 720;
					glfwSetWindowMonitor(app::window, NULL, 0, 30, 1280, 720, 0);
					fullscrean = false;
				}
				else {
					app::width = 1920;
					app::height = 1080;
					glfwSetWindowMonitor(app::window, glfwGetPrimaryMonitor(), 0, 0, 1980, 1080, 0);
					fullscrean = true;
				}
				glViewport(0, 0, width, height);
				projection = glm::perspective(glm::radians(110.0f), (float)width / (float)height, 0.0001f, 20000.0f);
			}
			hold = true;
		}
		else {
			hold = false;
		}
			
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			app::staticCam = true;
			app::firstMouse = true;
		}
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			app::staticCam = false;
		}
	}

	void mainLoop(float& dt) override {
		// boucle principale appel�e � chaque image

		inputs(dt);

		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		Cshader.useCompute(ssbo, floor(vertices.size() / 128.f));
		Cshader.setFloat("dt", dt);
		Cshader.setFloat("accuracy", accuracy);
		Cshader.setFloat("gravity", gravity);
		Cshader.setFloat("csmooth", csmooth);
		Cshader.setFloat("blackHole", blackHole);
		va.AddBuffer(ssbo,0, GL_ARRAY_BUFFER, 3, 12, 0);
		va.AddBuffer(ssbo, 1, GL_ARRAY_BUFFER, 3, 12, 4);
		shader.Bind();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setVec3("hue", hue[0], hue[1], hue[2]);

		renderer.DrawArray(va, shader, number);

		GUI(dt);
	}
};

int main(void){
	// debut du programme
	std::unique_ptr<Galaxy> galaxy = std::make_unique<Galaxy>();
	galaxy->start();
}