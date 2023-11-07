#include "Lab04.h"

#include "Application.h"
#include "Texture.h"
#include "Primitives.h"

#include "imgui.h"

#include "Input.h"
#include "UIHelpers.h"


std::vector<TransformTriangle> savedTransformTriangles;
std::vector<TransformIcosphere> savedTransformIcospheres;

// Renders to the "screen" texture that has been passed in as a parameter
void Lab04::render(s_ptr<Texture> screen) {

	auto mem = screen->memory;
	auto value = mem->value;
	auto pixels = (float*)mem->value;

	for (auto& tt : savedTransformTriangles) {
		if (!tt.triangle.enabled) continue;

		Triangle transformed = tt.getTransformedTri();
		
		renderTriangleBoundingBox(transformed, pixels, mem->stride, screen->resolution.x, screen->resolution.y);
	}

	double deltaTime = Application::get().deltaTime;

	for (auto& icoTf : savedTransformIcospheres) {
		if (!icoTf.enabled) continue;

		if (icoTf.transform.autoSpin) {
			icoTf.transform.rotation.w += deltaTime;
		}

		if (icoTf.transform.autoFall) {

			icoTf.transform.translation.y -= deltaTime * 30.0f;

			if (icoTf.transform.translation.y < 0) {
				icoTf.transform.translation.y = screen->resolution.y;
			}
		}

		Icosphere ico;

		mat4 M = icoTf.transform.getMatrix();

		vec2 zRange = vec2(0.0f);

		for (auto& pos : ico.positions) {
			vec4 p(pos.x, pos.y, pos.z, 1);
			p = M * p;
			pos = vec3(p);

			zRange.x = glm::min(zRange.x, pos.z);
			zRange.y = glm::max(zRange.y, pos.z);
		}

		// Renormalize the z-values so that they are all between 0 (closest, minimum depth) and 1 (farthest, maximum depth)
		float zDist = zRange.y - zRange.x - 1e-4f;

		for (auto& pos : ico.positions) {
			pos.z = 1e-4f + (pos.z - zRange.x) / zDist;
		}

		static bool useColors = true;
		static double lastChanged = 0;

		double totalTime = Application::get().timeSinceStart;

		if (Input::get().current.keyStates[GLFW_KEY_SPACE] && (totalTime - lastChanged) > 0.5) {
			useColors = !useColors;
			lastChanged = totalTime;
		}

		renderIcosphere(ico, pixels, mem->stride, screen->resolution.x, screen->resolution.y, useColors);
	}


	glBindTexture(GL_TEXTURE_2D, screen->id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen->resolution.x, screen->resolution.y, 0, GL_RGBA, GL_FLOAT, (const void*)pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Lab04::renderUI() {

	if (ImGui::Button("Load 1 icosphere")) {
		savedTransformIcospheres.push_back({
		{ 
			vec3(128, 64, 0),				// translation
			vec4(0.f),						// rotation center and amount
			vec3(25.0f, 25.0f, 25.0f)		// scale
		} });
	}

	ImGui::SameLine();

	if (ImGui::Button("Load 10 icospheres")) {
		srand(time(0));
		for (int i = 0; i < 10; i++) {
			savedTransformIcospheres.push_back({
			{
				// translation
				glm::linearRand(vec3(0.f), vec3(256, 128, 0.f)),	
				// rotation center and amount
				vec4(0.f),						
				// scale
				vec3(25.0f, 25.0f, 25.0f),
				// autoSpin
				true,
				RotationAxis::_from_integral(glm::linearRand<int>(0, 2)),
				true
			} });
		}
		
	}


	if (ImGui::CollapsingHeader("Transformable Icospheres") && !savedTransformIcospheres.empty()) {
		IMDENT;

		if (ImGui::Button("Clear all")) {
			savedTransformIcospheres.clear();
		}

		int counter = 1;

		auto icoToDelete = savedTransformIcospheres.end();
		//for (auto& line : savedLines) {
		for (auto it = savedTransformIcospheres.begin(); it != savedTransformIcospheres.end(); ++it) {
			auto& ico = *it;
			std::string icoLabel = fmt::format("Icosphere {0}", counter++);
			IMDENT;
			ImGui::PushID((const void*)&ico);
			ico.renderUI(icoLabel.c_str());
			ImGui::PopID();
			IMDONT;
			if (ico.shouldDelete) {
				icoToDelete = it;
			}
		}
		IMDONT;

		if (icoToDelete != savedTransformIcospheres.end()) {
			savedTransformIcospheres.erase(icoToDelete);
		}
	}
}
