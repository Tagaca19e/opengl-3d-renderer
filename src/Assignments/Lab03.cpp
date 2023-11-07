#include "Lab03.h"
#include "Texture.h"
#include "Primitives.h"

#include "imgui.h"
#include "UIHelpers.h"

TriangleRenderMode triangleRenderMode = TriangleRenderMode::Outline;

std::vector<Triangle> savedTriangles;

// Renders to the "screen" texture that has been passed in as a parameter
void Lab03::render(s_ptr<Texture> screen) {

	auto mem = screen->memory;
	auto value = mem->value;
	auto pixels = (float*)mem->value;

	for (auto& tri : savedTriangles) {
		if (!tri.enabled) continue;

		switch (triangleRenderMode) {
			case TriangleRenderMode::Outline:
				renderTriangleOutline(tri, pixels, mem->stride, screen->resolution.x, screen->resolution.y);
				break;
			case TriangleRenderMode::Parametric:
				renderTriangleParametric(tri, pixels, mem->stride, screen->resolution.x, screen->resolution.y);
				break;
			case TriangleRenderMode::BoundingBox:
				renderTriangleBoundingBox(tri, pixels, mem->stride, screen->resolution.x, screen->resolution.y);
				break;
			default:
				break;
		}
	}
	

	glBindTexture(GL_TEXTURE_2D, screen->id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen->resolution.x, screen->resolution.y, 0, GL_RGBA, GL_FLOAT, (const void*)pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Lab03::renderUI() {
	static vec3 p0 = { 0, 0, 0 };
	static vec3 p1 = { 0, 0, 0 };
	static vec3 p2 = { 0, 0, 0 };
	static vec4 color = { 1, 1, 1, 1 };

	ImGui::InputFloat3("P0", glm::value_ptr(p0));
	ImGui::InputFloat3("P1", glm::value_ptr(p1));
	ImGui::InputFloat3("P2", glm::value_ptr(p2));
	ImGui::ColorEdit4("Color", glm::value_ptr(color));

	if (ImGui::Button("Add triangle")) {
		Triangle newTri;
		newTri.vertices[0] = { p0, color };
		newTri.vertices[1] = { p1, color };
		newTri.vertices[2] = { p2, color };
		newTri.color = color;
		savedTriangles.push_back(newTri);
	}

	ImGui::SameLine();

	if (ImGui::Button("Load lab 03 triangles")) {
		savedTriangles.push_back(Triangle(vec3(128, 64, 0), vec3(160, 72, 0), vec3(128, 96, 0), vec3(173.0f / 255.0f)));
		savedTriangles.push_back(Triangle(vec3(128, 64, 0), vec3(128, 96, 0), vec3(96,  72, 0), vec3(137.0f / 255.0f)));
		savedTriangles.push_back(Triangle(vec3(128, 64, 0), vec3(96,  72, 0), vec3(108, 40, 0), vec3(90.0f  / 255.0f)));
		savedTriangles.push_back(Triangle(vec3(128, 64, 0), vec3(108, 41, 0), vec3(148, 41, 0), vec3(108.0f / 255.0f)));
		savedTriangles.push_back(Triangle(vec3(128, 64, 0), vec3(148, 41, 0), vec3(160, 72, 0), vec3(137.0f / 255.0f)));

		savedTriangles.push_back(Triangle(vec3(160, 72, 0), vec3(148, 88, 0), vec3(128, 96, 0), vec3(156.0f / 255.0f)));
		savedTriangles.push_back(Triangle(vec3(128, 96, 0), vec3(108, 88, 0), vec3(96,  72, 0), vec3(77.0f  / 255.0f)));
		savedTriangles.push_back(Triangle(vec3(96,  72, 0), vec3(96,  54, 0), vec3(108, 40, 0), vec3(77.0f  / 255.0f)));
		savedTriangles.push_back(Triangle(vec3(108, 40, 0), vec3(128, 34, 0), vec3(148, 40, 0), vec3(77.0f  / 255.0f)));
		savedTriangles.push_back(Triangle(vec3(148, 41, 0), vec3(160, 54, 0), vec3(160, 72, 0), vec3(118.0f / 255.0f)));

	}

	if (ImGui::CollapsingHeader("Triangles") && !savedTriangles.empty()) {
		IMDENT;

		if (ImGui::Button("Clear all")) {
			savedTriangles.clear();
		}

		renderEnumButton<TriangleRenderMode>(triangleRenderMode);

		int counter = 1;

		auto triToDelete = savedTriangles.end();
		//for (auto& line : savedLines) {
		for (auto it = savedTriangles.begin(); it != savedTriangles.end(); ++it) {
			auto& tri = *it;
			std::string triLabel = fmt::format("Triangle {0}", counter++);
			if (ImGui::CollapsingHeader(triLabel.c_str())) {
				IMDENT;
				ImGui::PushID((const void*)&tri);
				tri.renderUI();
				if (tri.shouldDelete) {
					triToDelete = it;
				}
				ImGui::PopID();
				IMDONT;
			}
		}
		IMDONT;

		if (triToDelete != savedTriangles.end()) {
			savedTriangles.erase(triToDelete);
		}
	}
}
