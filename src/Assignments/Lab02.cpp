#include "Lab02.h"
#include "Primitives.h"
#include "Texture.h"

#include "imgui.h"
#include "UIHelpers.h"

LineRenderMode lineRenderMode = LineRenderMode::Implicit;
ParametricLineMode parametricLineMode = ParametricLineMode::Samples;
//TriangleRenderMode triangleRenderMode = TriangleRenderMode::FromCoordinates;

float stepSize = 1.0f;

std::vector<Pixel> savedPixels;
std::vector<Line> savedLines;
std::vector<Circle> savedCircles;

float lineThickness = 1.0f;
float circleTolerance = 1.0f;

int debugX = 1000;
int debugY = 1000;

void renderLinesImplicit(float* pixels, int stride, int width, int height) {
	for (auto& line : savedLines) {
		renderLineImplicit(line, pixels, stride, width, height);
	}
}



void renderLinesParametric(float* pixels, int stride, int width, int height) {
	for (auto& line : savedLines) {
		renderLineParametric(line, pixels, stride, width, height);
	}
}

void renderCircles(float* pixels, int stride, int width, int height) {

	for (auto& circle : savedCircles) {
		renderCircle(circle, pixels, stride, width, height);
	}
}

bool lab2_initialized = false;

void lab2_init() {
	lab2_initialized = true;

	for (int i = 0; i < 60; i++) {
		int x = glm::linearRand<int>(0, 255);
		int y = glm::linearRand<int>(0, 127);
		savedPixels.push_back({ vec3(x, y, 0), vec4(1) });
	}
}

// Renders to the "screen" texture that has been passed in as a parameter
void Lab02::render(s_ptr<Texture> screen) {

	if (!lab2_initialized) lab2_init();

	auto mem = screen->memory;
	auto value = mem->value;
	auto pixels = (float*)mem->value;

	size_t numPixels = (size_t)screen->resolution.x * screen->resolution.y;

	for (size_t y = 0; y < screen->resolution.y; y++) {
		for (size_t x = 0; x < screen->resolution.x; x++) {

			size_t offset = ((y * screen->resolution.x) + x) * mem->stride;

			auto pixel = pixels + offset;

			pixel[0] = 0.0f;
			pixel[1] = 0.0f;
			pixel[2] = 0.0f;
			pixel[3] = 1.0f;
		}
	}

	// Render individual pixels
	for (auto& pixel : savedPixels) {
		if (pixel.position.x < 0 || pixel.position.y < 0 
			|| pixel.position.x >= screen->resolution.x || pixel.position.y >= screen->resolution.y) continue;

		int offset = ((pixel.position.y * screen->resolution.x) + pixel.position.x) * mem->stride;

		float * pixelLocation = pixels + offset;

		pixelLocation[0] = pixel.color.x;
		pixelLocation[1] = pixel.color.y;
		pixelLocation[2] = pixel.color.z;

	}

	// Render lines
	switch (lineRenderMode) {
		case LineRenderMode::Implicit:
			renderLinesImplicit(pixels, mem->stride, screen->resolution.x, screen->resolution.y);
			break;
		case LineRenderMode::Parametric:
			renderLinesParametric(pixels, mem->stride, screen->resolution.x, screen->resolution.y);
			break;
		default:
			break;
	}
	

	renderCircles(pixels, mem->stride, screen->resolution.x, screen->resolution.y);

	glBindTexture(GL_TEXTURE_2D, screen->id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen->resolution.x, screen->resolution.y, 0, GL_RGBA, GL_FLOAT, (const void*)pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Lab02::renderUI() {
	static vec3 p0 = { 0, 0, 0 };
	static vec3 p1 = { 0, 0, 0 };
	static vec3 p2 = { 0, 0, 0 };
	static vec4 color = { 1, 1, 1, 1 };

	ImGui::InputFloat3("P0", glm::value_ptr(p0));
	ImGui::InputFloat3("P1", glm::value_ptr(p1));
	ImGui::InputFloat3("P2", glm::value_ptr(p2));
	ImGui::ColorEdit4("Color", glm::value_ptr(color));
	ImGui::InputFloat("Line thickness", &lineThickness);
	ImGui::InputFloat("Circle tolerance", &circleTolerance);

	if (ImGui::Button("Add point")) {
		Pixel newPixel = { p0, color };
		savedPixels.push_back(newPixel);
	}

	if (ImGui::Button("Add line")) {
		Line newLine;
		newLine.p0 = { p0, color };
		newLine.p1 = { p1, color };
		newLine.thickness = lineThickness;
		newLine.color = color;
		savedLines.push_back(newLine);
	}

	if (ImGui::Button("Add circle")) {
		Circle newCircle;
		newCircle.center = { p0, color };
		newCircle.radius = p1.x;
		newCircle.tolerance = circleTolerance;

		savedCircles.push_back(newCircle);
	}

	if (ImGui::CollapsingHeader("Lines") && !savedLines.empty()) {
		IMDENT;

		ImGui::InputFloat("Step size", &stepSize);
		int counter = 1;
		renderEnumButton(lineRenderMode);
		renderEnumDropDown("ParametricLineMode", parametricLineMode);

		auto lineToDelete = savedLines.end();
		//for (auto& line : savedLines) {
		for (auto it = savedLines.begin(); it != savedLines.end(); ++it) {
			auto& line = *it;
			std::string lineLabel = fmt::format("Line {0}", counter++);
			if (ImGui::CollapsingHeader(lineLabel.c_str())) {
				IMDENT;
				ImGui::PushID((const void*)&line);
				line.renderUI();
				if (line.shouldDelete) {
					lineToDelete = it;
				}
				ImGui::PopID();
				IMDONT;
			}
		}
		IMDONT;

		if (lineToDelete != savedLines.end()) {
			savedLines.erase(lineToDelete);
		}
	}

	if (ImGui::CollapsingHeader("Circles") && !savedCircles.empty()) {
		IMDENT;
		int counter = 1;
		for (auto& circle : savedCircles) {
			std::string circleLabel = fmt::format("Circle {0}", counter++);
			if (ImGui::CollapsingHeader(circleLabel.c_str())) {
				IMDENT;
				ImGui::PushID((const void*)&circle);
				circle.renderUI();
				ImGui::PopID();
				IMDONT;
			}
			
		}
		IMDONT;
	}

	if (ImGui::CollapsingHeader("Debug")) {
		IMDENT;
		ImGui::InputInt("X", &debugX);
		ImGui::InputInt("Y", &debugY);
		IMDONT;
	}
}