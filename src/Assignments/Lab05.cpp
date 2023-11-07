#include "Lab05.h"

#include "Application.h"
#include "Texture.h"
#include "Primitives.h"

#include "imgui.h"

#include "Input.h"
#include "UIHelpers.h"


struct CelestialBody {
	std::string name;
	vec3 color = vec3(1);
	bool lightSource = false;
	bool enabled = true;
	bool shouldDelete = false;
	Transform2D transform;
	mat4 modelMatrix;
	bool useModelMatrix = false;
	bool autoOrbit = false;
	vec4 orbitalRotation = vec4(0);

	void renderUI() {
		ImGui::PushID((const void*)this);
		if (ImGui::CollapsingHeader(name.c_str())) {
			ImGui::Checkbox("Enabled", &enabled);
			ImGui::Checkbox("Light sources", &lightSource);
			ImGui::ColorEdit3("Color", glm::value_ptr(color));
			transform.renderUI();
			ImGui::Checkbox("Auto-orbit", &autoOrbit);
			ImGui::InputFloat3("Orbit on axis", glm::value_ptr(orbitalRotation));
			ImGui::SliderFloat("Orbital rotation", &orbitalRotation.w, 0, glm::two_pi<float>());

			if (ImGui::Button("Delete")) {
				shouldDelete = true;
			}
		}
		ImGui::PopID();
	}
};

std::vector<CelestialBody> celestialBodies;


vec3 cameraPosition(0, 0, 50);
vec3 cameraLookat(0, 0, 0);
vec3 cameraUp(0, 1, 0);
bool cameraOrtho = false;
float cameraFOVY = 60.0f;

vec2 nearFar(1, 1000);


vec3 WorldToScreen(vec3 world, mat4 modelview, mat4 projection, int viewportWidth, int viewportHeight)
{
	vec3 projected = glm::project(world, modelview, projection, vec4(0, 0, viewportWidth, viewportHeight));
	return projected;
}


void renderSphere(mat4 model, mat4 view, mat4 projection, vec3 color, float* pixels, int stride, int width, int height, vec3 * lightSource = nullptr) {
	mat4 mvp = projection * view * model;

	for (auto& triangleIndices : Sphere::instance.indices) {
		Triangle tri;

		for (int i = 0; i < 3; i++) {
			vec3 p = Sphere::instance.positions[triangleIndices[i]];
			tri.vertices[i].position = p;
			if (lightSource == nullptr) {
				tri.vertices[i].color = color;
				tri.vertices[i].position = WorldToScreen(p, view * model, projection, width, height);
			}
		}

		if (lightSource) {
			vec3 N = glm::normalize(tri.normal());

			N = model * vec4(N, 0);

			for (int i = 0; i < 3; i++) {
				vec3 p = model * vec4(tri.vertices[i].position, 1);
				vec3 L = glm::normalize(p - *lightSource);
				tri.vertices[i].color = color * glm::dot(N, L);
				tri.vertices[i].position = WorldToScreen(tri.vertices[i].position, view * model, projection, width, height);
			}
		}
		
		renderTriangleBoundingBox(tri, pixels, stride, width, height);
	}
}

vec3* getLightSource(const CelestialBody & body) {
	if (body.lightSource) {
		return nullptr;
	}

	for (auto& cb : celestialBodies) {
		if (cb.lightSource) {
			return &cb.transform.translation;
		}
	}

	return &cameraLookat;
}

void renderCelestialBody(const CelestialBody & body, float* pixels, int stride, int width, int height) {

	mat4 view = glm::lookAt(cameraPosition, cameraLookat, cameraUp);

	float fovy = glm::radians(cameraFOVY);
	float aspectRatio = float(width) / float(height);
	mat4 projection = glm::perspective(fovy, aspectRatio, nearFar.x, nearFar.y);

	if (cameraOrtho) {
		float halfWidth = width * 0.5f;
		float halfHeight = height * 0.5f;
		projection = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearFar.x, nearFar.y);
	}

	mat4 model = body.useModelMatrix ? body.modelMatrix : body.transform.getMatrixGLM();

	mat4 mvp = projection * view * model;

	renderSphere(model, view, projection, body.color, pixels, stride, width, height, getLightSource(body));
}

// TODO: edit the default values
void Lab05::init() {
	CelestialBody sunBody;
	sunBody.name = "Sun";
	sunBody.transform.translation = vec3(0, 0, 0);
	sunBody.transform.rotation = vec4(0, 0, 1, 0);
	sunBody.transform.scale = vec3(8);
	sunBody.color = vec3(1, 1, 0);
	sunBody.lightSource = true;

	celestialBodies.push_back(sunBody);

	CelestialBody planetBody;
	planetBody.name = "Planet";
	planetBody.transform.translation = vec3(24, 0, 0);
	planetBody.transform.rotation = vec4(0, 1, 0, 0);
	planetBody.transform.scale = vec3(3);
	planetBody.color = vec3(0.1, 0.1, 0.8);
	planetBody.useModelMatrix = true;
	planetBody.autoOrbit = true;
	planetBody.orbitalRotation = vec4(0, 1, 0, 0);
	
	celestialBodies.push_back(planetBody);

	// Extra credit: get a moon to orbit the planet as the planet orbits the sun
	CelestialBody moonBody;
	moonBody.name = "Moon";
	moonBody.transform.translation = vec3(16, 0, 0);
	moonBody.transform.rotation = vec4(0, 1, 0, 0);
	moonBody.transform.scale = vec3(4);
	moonBody.color = vec3(1.0f);
	moonBody.useModelMatrix = true;
	moonBody.autoOrbit = true;
	moonBody.orbitalRotation = vec4(0, -1, 0, 0);
	celestialBodies.push_back(moonBody);

	// Static object for referencing
	CelestialBody xAxis;
	xAxis.name = "xAxis";
	xAxis.lightSource = true;
	xAxis.transform.translation = vec3(20, 0, 0);
	xAxis.transform.rotation = vec4(0, 0, 1, 0);
	xAxis.transform.scale = vec3(20, 1, 1);
	xAxis.color = vec3(1, 0, 0);
	celestialBodies.push_back(xAxis);

	CelestialBody yAxis;
	yAxis.name = "yAxis";
	yAxis.lightSource = true;
	yAxis.transform.translation = vec3(0, 20, 0);
	yAxis.transform.rotation = vec4(0, 0, 1, 0);
	yAxis.transform.scale = vec3(1, 20, 1);
	yAxis.color = vec3(0, 1, 0);
	celestialBodies.push_back(yAxis);

	CelestialBody zAxis;
	zAxis.name = "zAxis";
	zAxis.lightSource = true;
	zAxis.transform.translation = vec3(0, 0, 20);
	zAxis.transform.rotation = vec4(0, 0, 1, 0);
	zAxis.transform.scale = vec3(1, 1, 20);
	zAxis.color = vec3(0, 0, 1);
	celestialBodies.push_back(zAxis);

	initialized = true;

}

// Renders to the "screen" texture that has been passed in as a parameter
void Lab05::render(s_ptr<Texture> screen) {
	if (!initialized) init();

	auto mem = screen->memory;
	auto value = mem->value;
	auto pixels = (float*)mem->value;

	double deltaTime = Application::get().deltaTime;
	double timeSinceStart = Application::get().timeSinceStart;

	vec3 planetPosition;

	if (celestialBodies.size() > 1) {
		CelestialBody& sun = celestialBodies[0];
		CelestialBody& planet = celestialBodies[1];
		
		// planet stuff
		{
			planet.transform.rotation.w += deltaTime * 3.0f;
			mat4 R = glm::rotate(planet.transform.rotation.w, vec3(planet.transform.rotation));

			mat4 S = glm::scale(planet.transform.scale);

			//R = postR * R * preR;
			if (planet.autoOrbit) {
				planet.orbitalRotation.w += deltaTime;
			}

			mat4 orbit = glm::rotate(planet.orbitalRotation.w, vec3(planet.orbitalRotation));

			planetPosition = orbit * vec4(planet.transform.translation, 1);


			planet.modelMatrix = glm::translate(planetPosition) * R * S;
		}

		// moon stuff
		if (celestialBodies.size() > 2) {
			CelestialBody& moon = celestialBodies[2];
			moon.transform.rotation.w += deltaTime * 3.0f;
			mat4 R = glm::rotate(moon.transform.rotation.w, vec3(moon.transform.rotation));

			mat4 S = glm::scale(moon.transform.scale);

			//R = postR * R * preR;
			if (moon.autoOrbit) {
				moon.orbitalRotation.w += deltaTime;
			}

			mat4 orbit = glm::rotate(moon.orbitalRotation.w, vec3(moon.orbitalRotation));

			vec3 moonPosition = orbit * vec4(moon.transform.translation, 1);


			moon.modelMatrix = 
				glm::translate(moonPosition)
				* glm::translate(planetPosition)
				* R 
				* S;
		}

	}

	for (auto& cb : celestialBodies) {
		if (!cb.enabled) continue;
		renderCelestialBody(cb, pixels, mem->stride, screen->resolution.x, screen->resolution.y);
	}

	glBindTexture(GL_TEXTURE_2D, screen->id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen->resolution.x, screen->resolution.y, 0, GL_RGBA, GL_FLOAT, (const void*)pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Lab05::renderUI() {

	if (ImGui::Button("Initialize Lab 05 icospheres")) {
		init();
	}

	if (ImGui::CollapsingHeader("Camera")) {
		ImGui::SliderFloat3("Position", glm::value_ptr(cameraPosition), -100, 100);
		ImGui::InputFloat3("Position manual", glm::value_ptr(cameraPosition));
		ImGui::SliderFloat3("Lookat", glm::value_ptr(cameraLookat), -100, 100);
		ImGui::InputFloat3("Lookat manual", glm::value_ptr(cameraLookat));
		ImGui::InputFloat3("Up", glm::value_ptr(cameraUp));
		ImGui::SliderFloat("FOVY", &cameraFOVY, 0, 360);
		ImGui::InputFloat2("Near-far", glm::value_ptr(nearFar));

		if (ImGui::Button(cameraOrtho ? "Orthographic" : "Perspective")) {
			cameraOrtho = !cameraOrtho;
		}
	}
	
	if (ImGui::CollapsingHeader("Celestial bodies") && !celestialBodies.empty()) {
		IMDENT;

		if (ImGui::Button("Clear all")) {
			celestialBodies.clear();
		}

		int counter = 1;

		auto cbToDelete = celestialBodies.end();
		for (auto it = celestialBodies.begin(); it != celestialBodies.end(); ++it) {
			auto& cb = *it;
			IMDENT;
			cb.renderUI();
			IMDONT;
			if (cb.shouldDelete) {
				cbToDelete = it;
			}
		}
		IMDONT;

		if (cbToDelete != celestialBodies.end()) {
			celestialBodies.erase(cbToDelete);
		}
	}
}
