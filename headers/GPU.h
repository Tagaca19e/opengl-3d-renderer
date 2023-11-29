#pragma once

#include "globals.h"
#include "Primitives.h"
#include "Renderer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct MeshVertex {
	vec3 position;
	vec3 color;
};

// The two mesh types need different vertex array and buffer objects, but they can
// use the same index buffer object. Your job will be to implement the cube indices
// inside this struct's init function.
struct IndexBuffer {
	std::vector<GLuint> indices;
	GLuint IBO = 0;

	// This is a helper function that can be called to add 3 indices
	// as a triangle in the cube mesh. 
	void addTriangle(GLuint v0, GLuint v1, GLuint v2) {
		indices.push_back(v0);
		indices.push_back(v1);
		indices.push_back(v2);
	}

	void init() {
		// Mesh vertices are generated as 
		// 0: -1, -1, -1		// left		bottom	back
		// 1: -1, -1, +1		// left		bottom	front
		// 2: -1, +1, -1		// left		top		back
		// 3: -1, +1, +1		// left		top		front
		// 4: +1, -1, -1		// right	bottom	back
		// 5: +1, -1, +1		// right	bottom	front
		// 6: +1, +1, -1		// right	top		back
		// 7: +1, +1, +1		// right	top		front

		// TODO: populate the indices 
		// 6 sides to a box, 2 triangles each: 12 triangles
		// 12 triangles, 3 indices each: 36 indices

		// Front triangles of box:
		addTriangle(1, 5, 7);
		addTriangle(7, 3, 1);

		// Right triangles of box
		addTriangle(5, 4, 6);
		addTriangle(6, 7, 5);

		// back triangles of box:
		addTriangle(4, 0, 2);
		addTriangle(2, 6, 4);

		// Left triangles of box:
		addTriangle(0, 1, 3);
		addTriangle(3, 2, 0);

		// Top triangles of box
		addTriangle(3, 7, 6);
		addTriangle(6, 2, 3);

		// Bottom triangles of box
		addTriangle(0, 4, 5);
		addTriangle(5, 1, 0);

		glGenBuffers(1, &IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	}
};

struct TriangleMeshSOA {
	std::vector<GLfloat> positions;
	std::vector<GLfloat> colors;

	GLuint VAO = 0;
	GLuint VBO = 0;
	GLuint CBO = 0;

	TriangleMeshSOA() {

		Icosphere sphere;

		for (auto& pos : sphere.positions) {
			positions.push_back(pos.x);
			positions.push_back(pos.y);
			positions.push_back(pos.z);
		}

		for (auto& color : sphere.colors) {
			colors.push_back(color.x);
			colors.push_back(color.y);
			colors.push_back(color.z);
		}

		// Generates vertex positions and colors using nested for loop. Leave this code alone.
		//for (int x = -1; x <= 1; x += 2) {
		//	for (int y = -1; y <= 1; y += 2) {
		//		for (int z = -1; z <= 1; z += 2) {
		//			vec3 position(x, y, z);
		//			vec3 color = (position + vec3(1)) * 0.5f;

		//			log("SOA Adding vertex {0}: {1}\n", positions.size(), glm::to_string(position));

		//			positions.push_back(position.x);
		//			positions.push_back(position.y);
		//			positions.push_back(position.z);

		//			colors.push_back(color.x);
		//			colors.push_back(color.y);
		//			colors.push_back(color.z);
		//		}
		//	}
		//}
	}

	// Initializes the GPU data for the SAO triangle mesh. Already in working order.
	void init(GLuint IBO, GLuint shaderProgram) {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);

		// Now that we have multiple vertex attributes, it's less safe to rely on hardcoded 0 or 1 locations. 
		// Instead, look up the attribute's location for the shader program. 
		GLint positionLoc = glGetAttribLocation(shaderProgram, "vPos");
		glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(positionLoc);

		glGenBuffers(1, &CBO);
		glBindBuffer(GL_ARRAY_BUFFER, CBO);
		glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);

		GLint colorLoc = glGetAttribLocation(shaderProgram, "vCol");
		glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(colorLoc);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

		glBindVertexArray(0);
	}
};

struct TriangleMeshAOS {
	std::vector<MeshVertex> vertices;

	GLuint VAO = 0;
	GLuint VBO = 0;

	// Generates vertex positions and colors using nested for loop. Leave this code alone.
	TriangleMeshAOS() {
		for (int x = -1; x <= 1; x += 2) {
			for (int y = -1; y <= 1; y += 2) {
				for (int z = -1; z <= 1; z += 2) {
					vec3 pos(x, y, z);
					MeshVertex mv;
					mv.position = pos;
					mv.color = (pos + vec3(1)) * 0.5f;

					log("AOS Adding vertex {0}: {1}\n", vertices.size(), glm::to_string(mv.position));

					vertices.push_back(mv);
				}
			}
		}
	}

	// TODO: generate and bind the VAO, then handle the VBO, then set up the vertex attribute pointers, then
	// bind the passed IBO. Look at TriangleMeshSAO::init as an outline, but know that an exact copy
	// of the other mesh's code won't work here. You'll need to modify it. 
	void init(GLuint IBO, GLuint shaderProgram) {

	}
};

struct Shader {
	GLuint vertexShader = 0;
	GLuint fragmentShader = 0;
	GLuint tessControlShader = 0;
	GLuint tessEvalShader = 0;
	GLuint program = 0;

	bool compileShader(const char* shaderSrc, GLenum shaderType, GLuint& shader) {
		shader = glCreateShader(shaderType);

		std::string shaderText = StringUtil::replaceAll(shaderSrc, "__VERSION__", SoftwareRenderer::shaderVersion);

		const char* shaderTextPtr = shaderText.c_str();
		glShaderSource(shader, 1, &shaderTextPtr, nullptr);
		glCompileShader(shader);

		int  success = 0;
		static char infoLog[512];

		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		if (!success) {
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			log("{0} shader compilation failed:\n{1}\n", (shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment"), infoLog);
			return false;
		}

		return true;
	}

	bool linkProgram() {
		GLint success = 0;

		program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);

		if (tessControlShader != 0) {
			glAttachShader(program, tessControlShader);
		}

		if (tessEvalShader != 0) {
			glAttachShader(program, tessEvalShader);
		}

		glLinkProgram(program);
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(program, 512, nullptr, infoLog);
			log("Shader linking failed:\n{0}\n", infoLog);
			return false;
		}

		return true;
	}

	bool init(const char* vertexShaderSrc, const char* fragmentShaderSrc,
		const char* tessControlShaderSrc = nullptr, const char* tessEvalShaderSrc = nullptr) {

		if (!compileShader(vertexShaderSrc, GL_VERTEX_SHADER, vertexShader)) {
			if (vertexShader != 0) {
				glDeleteShader(vertexShader);
				vertexShader = 0;
			}
			return false;
		}

		if (!compileShader(fragmentShaderSrc, GL_FRAGMENT_SHADER, fragmentShader)) {
			if (fragmentShader != 0) {
				glDeleteShader(fragmentShader);
				fragmentShader = 0;
			}
			return false;
		}

		if (tessControlShaderSrc != nullptr) {
			if (!compileShader(tessControlShaderSrc, GL_TESS_CONTROL_SHADER, tessControlShader)) {
				if (tessControlShader != 0) {
					glDeleteShader(tessControlShader);
					tessControlShader = 0;
				}
				return false;
			}
		}

		if (tessEvalShaderSrc != nullptr) {
			if (!compileShader(tessEvalShaderSrc, GL_TESS_EVALUATION_SHADER, tessEvalShader)) {
				if (tessEvalShader != 0) {
					glDeleteShader(tessEvalShader);
					tessEvalShader = 0;
				}
				return false;
			}
		}

		if (!linkProgram()) {
			if (program != 0) {
				glDeleteProgram(program);
				program = 0;
			}
			return false;
		}

		return true;
	}
};

struct Lighting {
	// Ambient terms
	float Ia = 0.05f;		// intensity
	vec3 Ka = vec3(1.f);	// color

	// Diffuse terms
	float Id = 1.0f;
	vec3 Kd = vec3(1.f);

	// Specular terms
	float shininess = 32.f;
	vec3 Ks = vec3(1.f);

	vec3 lightDirection = vec3(0, -1, 0);

	bool autoOrbit = false;

	vec3 orbitAxis = vec3(0, 1, 0);

	bool point = false;
	vec3 position = vec3(0.f);
	float intensity = 10.f;
	float radius = 2.0f;

	void renderUI() {
		if (ImGui::CollapsingHeader("Lighting")) {
			ImGui::InputFloat("Ambient intensity", &Ia);
			ImGui::ColorEdit3("Ambient color", glm::value_ptr(Ka));
			ImGui::InputFloat("Diffuse intensity", &Id);
			ImGui::ColorEdit3("Diffuse color", glm::value_ptr(Kd));
			ImGui::InputFloat("Shininess", &shininess);
			ImGui::ColorEdit3("Specular color", glm::value_ptr(Ks));
			ImGui::InputFloat3("Light direction", glm::value_ptr(lightDirection));
			ImGui::Checkbox("Point light", &point);
			if (point) {
				ImGui::InputFloat3("Position", glm::value_ptr(position));
				ImGui::InputFloat("Intensity", &intensity);
				ImGui::InputFloat("Radius", &radius);
			}

			ImGui::Checkbox("Auto-orbit", &autoOrbit);
			ImGui::InputFloat3("Orbit on axis", glm::value_ptr(orbitAxis));
		}
	}
};