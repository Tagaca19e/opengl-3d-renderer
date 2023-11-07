#include "Lab07.h"

#include "Application.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "GPU.h"
#include "Input.h"
#include "Primitives.h"
#include "Renderer.h"
#include "StringUtil.h"
#include "Texture.h"
#include "UIHelpers.h"

#include "imgui.h"

// TODO: edit the default values
void Lab07::init() {

	indexBuffer.init();

	const char* lab07VertexShaderSrc = R"VERTEXSHADER(
__VERSION__

uniform mat4 mvp;
uniform mat4 model;
uniform mat4 normalMatrix;

in vec3 vPos;
in vec3 vCol;

out vec3 fCol;
out vec3 fPos;
out vec3 fNormal;

void main() {
	fCol = vCol;
	fPos = (model * vec4(vPos, 1.0)).xyz;
	fNormal = (normalMatrix * vec4(vPos, 0.0)).xyz;
	
	gl_Position = mvp * vec4(vPos.x, vPos.y, vPos.z, 1.0);
}

)VERTEXSHADER";

	const char* lab07FragmentShaderSrc = R"FRAGMENTSHADER(
__VERSION__

uniform vec4 color;

uniform float Ia;
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float shininess;

uniform vec3 viewPosition;
uniform vec3 lightDirection;

uniform int useBlinn = 0;

in vec3 fCol;
in vec3 fPos;
in vec3 fNormal;
in float intensity;

out vec4 FragColor;

void main() {

	vec3 normal = normalize(fNormal);
	vec3 ambient = Ia * Ka;
	vec3 diffuse = Kd * max(0.0, dot(normal, -lightDirection));

	vec3 viewDir = normalize(viewPosition - fPos);
	vec3 reflectDir = reflect(lightDirection, normal);

	vec3 halfVector = normalize(-lightDirection + viewDir);

	float spec = 0.0;
	if (useBlinn > 0) {
		spec = pow(max(dot(normal, halfVector), 0.0), shininess);
	}
	else {
		spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	}

	vec3 specular = spec * Ks;
	
	vec3 finalColor = vec3(0.);
	finalColor = (ambient + diffuse + specular) * color.rgb * fCol;
	FragColor = vec4(finalColor, 1.0);
}

)FRAGMENTSHADER";

	if (!shader.init(lab07VertexShaderSrc, lab07FragmentShaderSrc)) {
		exit(1);
	}

	cubeSOA.init(indexBuffer.IBO, shader.program);
	cubeAOS.init(indexBuffer.IBO, shader.program);

	addSceneObject();

	initialized = true;
}

// Renders to the "screen" texture that has been passed in as a parameter
void Lab07::render(s_ptr<Framebuffer> framebuffer) {
	if (!initialized) init();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	double deltaTime = Application::get().deltaTime;
	double timeSinceStart = Application::get().timeSinceStart;

	glUseProgram(shader.program);
	if (renderSOA) {
		glBindVertexArray(cubeSOA.VAO);
	}
	else {
		glBindVertexArray(cubeAOS.VAO);
	}

	camera.update(float(framebuffer->width), float(framebuffer->height));

	mat4 vp = camera.projection * camera.view;

	for (auto& sceneObject : sceneObjects) {
		if (glm::length(vec3(sceneObject.transform.rotation)) > 0.f) {
			sceneObject.transform.rotation.w += deltaTime;
		}
		mat4 model = sceneObject.transform.getMatrixGLM();
		mat4 mvp = vp * model;

		GLuint mvpLocation = glGetUniformLocation(shader.program, "mvp");
		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));

		GLuint colorLocation = glGetUniformLocation(shader.program, "color");
		glUniform4fv(colorLocation, 1, glm::value_ptr(sceneObject.color));

		glDrawElements(GL_TRIANGLES, indexBuffer.indices.size(), GL_UNSIGNED_INT, 0);
	}
}

void Lab07::renderUI() {

	if (ImGui::Button(renderSOA ? "Struct of arrays" : "Array of structs")) {
		renderSOA = !renderSOA;
	}

	camera.renderUI();

	if (ImGui::CollapsingHeader("Scene objects")) {
		IMDENT;

		static int numberToAdd = 10;

		if (ImGui::Button("Add new")) {
			addSceneObject();
		}
		ImGui::SetNextItemWidth(200);
		ImGui::InputInt("Number to add", &numberToAdd);
		ImGui::SameLine();
		std::string addRandomLabel = fmt::format("Add {0}", numberToAdd);
		if (ImGui::Button(addRandomLabel.c_str())) {
			for (int i = 0; i < glm::max(numberToAdd, 0); i++) {
				SceneObject newObject;
				newObject.name = fmt::format("Object {0}", sceneObjects.size() + 1);

				newObject.transform.translation = glm::linearRand(vec3(-200), vec3(200));
				newObject.transform.rotation = vec4(glm::sphericalRand(1.0f), glm::linearRand(0.f, glm::two_pi<float>()));
				newObject.transform.scale = vec3(glm::linearRand(0.1f, 8.f));
				newObject.color = glm::linearRand(vec3(0.1f), vec3(1.f));
				
				sceneObjects.push_back(newObject);
			}
		}

		if (sceneObjects.size() > 0) {
			if (ImGui::Button("Clear all")) {
				sceneObjects.clear();
			}

			int counter = 1;

			ImGui::Text("Number of objects: %lu", sceneObjects.size());

			auto soToDelete = sceneObjects.end();
			for (auto it = sceneObjects.begin(); it != sceneObjects.end() && counter++ < 100; ++it) {
				auto& cb = *it;
				IMDENT;
				cb.renderUI();
				IMDONT;
				if (cb.shouldDelete) {
					soToDelete = it;
				}
			}

			if (soToDelete != sceneObjects.end()) {
				sceneObjects.erase(soToDelete);
			}
		}

		IMDONT;
	}
}
