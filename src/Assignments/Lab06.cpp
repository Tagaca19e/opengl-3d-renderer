#include "Lab06.h"

#include "Application.h"
#include "Texture.h"
#include "Primitives.h"
#include "Framebuffer.h"

#include "imgui.h"

#include "Input.h"
#include "UIHelpers.h"
#include "StringUtil.h"
#include "Renderer.h"

// Same as celestial bodies from lab 05
std::vector<SceneObject> sceneObjectsLab06;

struct Lab06Camera {
	vec3 cameraPosition = vec3(0, 0, 5);
	vec3 cameraLookat = vec3(0, 0, 0);
	vec3 cameraUp = vec3(0, 1, 0);
	bool cameraOrtho = false;
	float cameraFOVY = 60.0f;
	vec2 nearFar = vec2(1, 1000);
};

Lab06Camera camera;

// TODO: initialize the OpenGL objects
void Lab06::init() {
	initialized = true;
}

// Renders objects using the framebuffer
void Lab06::render(s_ptr<Framebuffer> framebuffer) {
	if (!initialized) init();

	for (const auto& sceneObject : sceneObjectsLab06) {
		// TODO: render each scene object
	}
}

void Lab06::renderUI() {

	if (ImGui::Button("Initialize Lab 06")) {
		init();
	}

	if (ImGui::CollapsingHeader("Camera")) {
		ImGui::SliderFloat3("Position", glm::value_ptr(camera.cameraPosition), -100, 100);
		ImGui::InputFloat3("Position manual", glm::value_ptr(camera.cameraPosition));
		ImGui::SliderFloat3("Lookat", glm::value_ptr(camera.cameraLookat), -100, 100);
		ImGui::InputFloat3("Lookat manual", glm::value_ptr(camera.cameraLookat));
		ImGui::InputFloat3("Up", glm::value_ptr(camera.cameraUp));
		ImGui::SliderFloat("FOVY", &camera.cameraFOVY, 0, 360);
		ImGui::InputFloat2("Near-far", glm::value_ptr(camera.nearFar));

		if (ImGui::Button(camera.cameraOrtho ? "Orthographic" : "Perspective")) {
			camera.cameraOrtho = !camera.cameraOrtho;
		}
	}

	if (ImGui::CollapsingHeader("Scene objects")) {
		IMDENT;

		static int numberToAdd = 10;

		if (ImGui::Button("Add new")) {
			SceneObject newObject;
			newObject.name = fmt::format("Object {0}", sceneObjectsLab06.size() + 1);
			sceneObjectsLab06.push_back(newObject);
		}
		ImGui::SetNextItemWidth(200);
		ImGui::InputInt("Number to add", &numberToAdd);
		ImGui::SameLine();
		std::string addRandomLabel = fmt::format("Add {0}", numberToAdd);
		if (ImGui::Button(addRandomLabel.c_str())) {
			for (int i = 0; i < glm::max(numberToAdd, 0); i++) {
				SceneObject newObject;
				newObject.name = fmt::format("Object {0}", sceneObjectsLab06.size() + 1);

				newObject.transform.translation = glm::linearRand(vec3(-200), vec3(200));
				newObject.transform.scale = vec3(glm::linearRand(0.1f, 8.f));
				newObject.color = glm::linearRand(vec3(0.1f), vec3(1.f));
				sceneObjectsLab06.push_back(newObject);
			}
		}

		if (sceneObjectsLab06.size() > 0) {
			if (ImGui::Button("Clear all")) {
				sceneObjectsLab06.clear();
			}

			int counter = 1;

			ImGui::Text("Number of objects: %lu", sceneObjectsLab06.size());

			auto soToDelete = sceneObjectsLab06.end();
			for (auto it = sceneObjectsLab06.begin(); it != sceneObjectsLab06.end() && counter++ < 100; ++it) {
				auto& cb = *it;
				IMDENT;
				cb.renderUI();
				IMDONT;
				if (cb.shouldDelete) {
					soToDelete = it;
				}
			}

			if (soToDelete != sceneObjectsLab06.end()) {
				sceneObjectsLab06.erase(soToDelete);
			}
		}
		
		IMDONT;
	}
}
