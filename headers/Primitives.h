#pragma once

#include "globals.h"
#include "imgui.h"
#include "UIHelpers.h"

// Some functions that will be common to assignments (or from old assignments that get promoted to the global codebase)
MAKE_ENUM(LineRenderMode, int, Implicit, Parametric)
MAKE_ENUM(ParametricLineMode, int, Samples, Midpoint, Brenesam, DDA)
MAKE_ENUM(TriangleRenderMode, int, Outline, Parametric, BoundingBox)

// Vertex: represents a single point in space. Has attributes that a rasterizer
// or ray-tracer could interpolate for rendering purposes. 
struct Vertex {
	vec3 position = vec3(0.f);
	vec3 color = vec3(1.f);

	void renderUI(const char* label = "Vertex") {
		ImGui::PushID((const void*)this);
		if (ImGui::CollapsingHeader(label)) {
			IMDENT;
			ImGui::InputFloat2("Position", glm::value_ptr(position));
			ImGui::ColorEdit3("Color", glm::value_ptr(color));
			IMDONT;
		}
		ImGui::PopID();
	}
};

using Pixel = Vertex;

// Line: a primitive between 2 vertices. 
struct Line {
	Vertex p0;
	Vertex p1;
	vec3 color = vec3(1.f);
	float thickness = 1.0f;
	bool enabled = true;
	bool shouldDelete = false;

	// Returns the distance between a point P and the line between p0 and p1.
	// The parameterized distance is assigned to the reference parameter t so 
	// the calling function can use the value for interpolation. 
	float dist(vec2 p, float& t) const {
		// h: the vector representing the hypotenuse of the triangle between p0, p, and the closest point on the line to p
		vec2 h = p - vec2(p0.position);
		// r: the ray of the line
		vec2 r = p1.position - p0.position;

		// The dot product between h and r is the squared length of h's projection onto r.
		// Dividing it by r gives us a parameterized value t representing how "far along"
		// point p is along the line segment. For p to be "in between" p0 and p1, then 
		// t must be in [0, 1].
		t = glm::dot(h, r) / glm::length2(r);

		// But t could be past either end of the line, so we do distance checks to p0 and p1, 
		// the closest points on the line to p in either case.
		if (t < 0) {
			return glm::length(p - vec2(p0.position));
		}
		else if (t > 1) {
			return glm::length(p - vec2(p1.position));
		}

		// pp is the closest point on the line to p
		vec2 pp = vec2(p0.position) + t * r;

		// And so we can take the length of p - pp as the distance. 
		return glm::length(p - pp);
	}

	void renderUI() {
		ImGui::Checkbox("Enabled", &enabled);
		p0.renderUI("P0");
		p1.renderUI("P1");
		ImGui::InputFloat("Thickness", &thickness);
		if (ImGui::Button("Delete")) {
			shouldDelete = true;
		}
	}
};

struct Triangle {
	Vertex vertices[3];

	vec3 color = vec3(1.f);
	bool enabled = true;
	bool shouldDelete = false;

	Vertex& p0() { return vertices[0]; }
	Vertex& p1() { return vertices[1]; }
	Vertex& p2() { return vertices[2]; }

	vec3 normal() {
		return glm::cross(p2().position - p0().position, p1().position - p0().position);
	}

	Triangle() { }
	Triangle(const vec3& v1, const vec3& v2, const vec3& v3, const vec3& col) {
		vertices[0] = { v1, vec3(1.f) };
		vertices[1] = { v2, vec3(1.f) };
		vertices[2] = { v3, vec3(1.f) };
		color = col;
	}

	// Computes the barycentric coordinates for point p such that
	// p = bary.x * p0 + bary.y * p1 + bary.z * p2;
	// Works in 2D.

	// Uses the triangle positions as non-orthogonal basis for point p.
	// In matrix form, (p - p0) = (p1 - p0, p2 - p0) * b
	vec3 barycentric(vec3 p) {
		vec2 P = vec2(p) - vec2(p0().position);
		mat2 R;
		R[0] = vec2(p1().position - p0().position);
		R[1] = vec2(p2().position - p0().position);

		vec2 L = glm::inverse(R) * P;

		return vec3(1.0f - L.x - L.y, L.x, L.y);
	}

	Vertex computeFromBarycentric(vec3 bary) {
		Vertex result = { vec3(0.f), vec3(0.f) };

		for (int i = 0; i < 3; i++) {
			result.position += bary[i] * vertices[i].position;
			result.color += bary[i] * vertices[i].color;
		}

		return result;
	}

	bool baryInTriangle(vec3 bary) {
		float sum = 0.0f;
		for (int i = 0; i < 3; i++) {
			if (bary[i] < 0.0f || bary[i] > 1.0f) return false;
			sum += bary[i];
		}

		if (sum < 0.99f || sum > 1.01f) return false;

		return true;
	}

	void renderUI() {
		ImGui::Checkbox("Enabled", &enabled);
		vertices[0].renderUI("P0");
		vertices[1].renderUI("P1");
		vertices[2].renderUI("P2");
		ImGui::ColorEdit3("Color", glm::value_ptr(color));
		if (ImGui::Button("Delete")) {
			shouldDelete = true;
		}
	}
};

struct Icosphere {
	vec3 positions[12] = {
		{  0,		 -1,		 0		  },
		{  0.723600, -0.447215,	 0.525720 },
		{ -0.276385, -0.447215,	 0.850640 },
		{ -0.894425, -0.447215,	 0.000000 },
		{ -0.276385, -0.447215,	-0.850640 },
		{  0.723600, -0.447215,	-0.525720 },
		{  0.276385,  0.447215,	 0.850640 },
		{ -0.723600,  0.447215,	 0.525720 },
		{ -0.723600,  0.447215,	-0.525720 },
		{  0.276385,  0.447215,	-0.850640 },
		{  0.894425,  0.447215,	 0.000000 },
		{  0,		  1,		 0		  }
	};

	vec3 colors[12] = {
		vec3(47, 79, 79) / 255.0f,
		vec3(139, 69, 19) / 255.0f,
		vec3(34, 139, 34) / 255.0f,
		vec3(0, 0, 139) / 255.0f,
		vec3(255, 0, 0) / 255.0f,
		vec3(255, 215, 0) / 255.0f,
		vec3(127, 255, 0) / 255.0f,
		vec3(0, 255, 255) / 255.0f,
		vec3(255, 0, 255) / 255.0f,
		vec3(100, 149, 237) / 255.0f,
		vec3(255, 105, 180) / 255.0f,
		vec3(255, 228, 196) / 255.0f,
	};

	ivec3 indices[20] = {
		{  0,  1,  2  },
		{  1,  0,  5  },
		{  0,  2,  3  },
		{  0,  3,  4  },
		{  0,  4,  5  },
		{  1,  5,  10 },
		{  2,  1,  6  },
		{  3,  2,  7  },
		{  4,  3,  8  },
		{  5,  4,  9  },
		{  1, 10,  6  },
		{  2,  6,  7  },
		{  3,  7,  8  },
		{  4,  8,  9  },
		{  5,  9,  10 },
		{  6, 10,  11 },
		{  7,  6,  11 },
		{  8,  7,  11 },
		{  9,  8,  11 },
		{ 10,  9,  11 }
	};
};

struct Sphere {
	std::vector<vec4> positions = {
		{ 0, 0, 0, 1 },
		{ 0.00000, -1.00000, 0.00000, 1  },
		{ 0.72360, -0.44722, 0.525725, 1 },
		{ -0.27638, -0.44722, 0.850649, 1 },
		{ -0.89442, -0.44721, 0.000000, 1 },
		{ -0.27638, -0.44722, -0.850649, 1 },
		{ 0.72360, -0.44722, -0.525725, 1 },
		{ 0.27638, 0.44722, 0.850649, 1 },
		{ -0.72360, 0.44722, 0.525725, 1 },
		{ -0.72360, 0.44722, -0.525725, 1 },
		{ 0.27638, 0.44722, -0.850649, 1},
		{ 0.89442, 0.44721, 0.000000, 1 },
		{ 0.00000, 1.00000, 0.000000, 1 },
		{ -0.16245, -0.85065, 0.499995, 1 },
		{ 0.42532, -0.85065, 0.309011, 1 },
		{ 0.26286, -0.52573, 0.809012, 1 },
		{ 0.85064, -0.52573, 0.000000, 1 },
		{ 0.42532, -0.85065, -0.309011, 1 },
		{ -0.52573, -0.85065, 0.000000, 1 },
		{ -0.68818, -0.52573, 0.499997, 1 },
		{ -0.16245, -0.85065, -0.499995, 1 },
		{ -0.68818, -0.52573, -0.499997, 1 },
		{ 0.26286, -0.52573, -0.809012, 1 },
		{ 0.95105, 0.00000, 0.309013, 1 },
		{ 0.95105, 0.00000, -0.309013, 1 },
		{ 0.00000, 0.00000, 1.000000, 1 },
		{ 0.58778, 0.00000, 0.809017, 1 },
		{ -0.95105, 0.00000, 0.309013, 1 },
		{ -0.58778, 0.00000, 0.809017, 1 },
		{ -0.58778, 0.00000, -0.809017, 1 },
		{ -0.95105, 0.00000, -0.309013, 1 },
		{ 0.58778, 0.00000, -0.809017, 1 },
		{ 0.00000, 0.00000, -1.000000, 1 },
		{ 0.68818, 0.52573, 0.499997, 1 },
		{ -0.26286, 0.52573, 0.809012, 1 },
		{ -0.85064, 0.52573, 0.000000, 1 },
		{ -0.26286, 0.52573, -0.809012, 1 },
		{ 0.68818, 0.52573, -0.499997, 1 },
		{ 0.16245, 0.85065, 0.499995, 1 },
		{ 0.52573, 0.85065, 0.000000, 1 },
		{ -0.42532, 0.85065, 0.309011, 1 },
		{ -0.42532, 0.85065, -0.309011, 1 },
		{ 0.16245, 0.85065, -0.499995, 1 },
	};

	std::vector<uvec3> indices = {
		{ 1,  14,  13 },
		{ 2,  14,  16 },
		{ 1,  13,  18 },
		{ 1,  18,  20 },
		{ 1,  20,  17 },
		{ 2,  16,  23 },
		{ 3,  15,  25 },
		{ 4,  19,  27 },
		{ 5,  21,  29 },
		{ 6,  22,  31 },
		{ 2,  23,  26 },
		{ 3,  25,  28 },
		{ 4,  27,  30 },
		{ 5,  29,  32 },
		{ 6,  31,  24 },
		{ 7,  33,  38 },
		{ 8,  34,  40 },
		{ 9,  35,  41 },
		{ 10,  36,  42 },
		{ 11,  37,  39 },
		{ 39,  42,  12 },
		{ 39,  37,  42 },
		{ 37,  10,  42 },
		{ 42,  41,  12 },
		{ 42,  36,  41 },
		{ 36,  9,  41 },
		{ 41,  40,  12 },
		{ 41,  35,  40 },
		{ 35,  8,  40 },
		{ 40,  38,  12 },
		{ 40,  34,  38 },
		{ 34,  7,  38 },
		{ 38,  39,  12 },
		{ 38,  33,  39 },
		{ 33,  11,  39 },
		{ 24,  37,  11 },
		{ 24,  31,  37 },
		{ 31,  10,  37 },
		{ 32,  36,  10 },
		{ 32,  29,  36 },
		{ 29,  9,  36 },
		{ 30,  35,  9 },
		{ 30,  27,  35 },
		{ 27,  8,  35 },
		{ 28,  34,  8 },
		{ 28,  25,  34 },
		{ 25,  7,  34 },
		{ 26,  33,  7 },
		{ 26,  23,  33 },
		{ 23,  11,  33 },
		{ 31,  32,  10 },
		{ 31,  22,  32 },
		{ 22,  5,  32 },
		{ 29,  30,  9 },
		{ 29,  21,  30 },
		{ 21,  4,  30 },
		{ 27,  28,  8 },
		{ 27,  19,  28 },
		{ 19,  3,  28 },
		{ 25,  26,  7 },
		{ 25,  15,  26 },
		{ 15,  2,  26 },
		{ 23,  24,  11 },
		{ 23,  16,  24 },
		{ 16,  6,  24 },
		{ 17,  22,  6 },
		{ 17,  20,  22 },
		{ 20,  5,  22 },
		{ 20,  21,  5 },
		{ 20,  18,  21 },
		{ 18,  4,  21 },
		{ 18,  19,  4 },
		{ 18,  13,  19 },
		{ 13,  3,  19 },
		{ 16,  17,  6 },
		{ 16,  14,  17 },
		{ 14,  1,  17 },
		{ 13,  15,  3 },
		{ 13,  14,  15 },
		{ 14,  2,  15 },
	};

	static Sphere instance;
};

struct Circle {
	Vertex center;
	float radius;
	float tolerance;

	float dist(vec2 coordinate) const {
		vec2 diff = coordinate - vec2(center.position);

		return glm::length(diff);
	}

	void renderUI() {
		ImGui::InputFloat2("Center", glm::value_ptr(center.position));
		ImGui::InputFloat("Radius", &radius);
		ImGui::InputFloat("Tolerance", &tolerance);
		ImGui::ColorEdit3("Color", glm::value_ptr(center.color));
	}
};

MAKE_ENUM(RotationAxis, int, X, Y, Z);

struct Transform2D {
	// Translation: offsets to X and Y position
	vec3 translation = vec3(0.f);
	// Rotation: rotates point around center of rotation (X, Y, Z) by angle (W)
	vec4 rotation = vec4(0.f);
	// Scale: resizes based on width (X) and height (Y) and depth (Z)
	vec3 scale = vec3(1.f);

	bool autoSpin = false;
	RotationAxis axis = RotationAxis::Z;

	bool autoFall = false;

	// Computes a 4x4 matrix that can transform 2D and 3D points by the translation, rotation, and scale listed above
	mat4 getMatrix() {
		mat4 S;
		S[0] *= scale.x;
		S[1] *= scale.y;
		S[2] *= scale.z;

		mat4 T;
		T[3] = vec4(translation, 1);

		mat4 R;
		if (rotation.w != 0) {

			float cosR = glm::cos(rotation.w);
			float sinR = glm::sin(rotation.w);

			switch (axis) {
			case RotationAxis::X:
				R[1] = vec4(0, cosR, sinR, 0);
				R[2] = vec4(0, -sinR, cosR, 0);
				break;
			case RotationAxis::Y:
				R[0] = vec4(cosR, 0, -sinR, 0);
				R[2] = vec4(sinR, 0, cosR, 0);
				break;
			case RotationAxis::Z:
				R[0] = vec4(cosR, sinR, 0, 0);
				R[1] = vec4(-sinR, cosR, 0, 0);
				break;
			default:
				break;
			}

			// Off-center rotations need to be translated to the center, then away

			if (glm::length(vec2(rotation)) > 0.f) {
				mat4 preT;
				preT[3] = vec4(-vec3(rotation) * scale, 1);
				mat4 posT;
				posT[3] = vec4(vec3(rotation) * scale, 1);

				R = posT * R * preT;
			}
		}

		mat4 M = T * R * S;

		return M;
	}

	mat4 getMatrixGLM() const {
		mat4 S = glm::scale(scale);

		mat4 T = glm::translate(translation);
		
		mat4 R;
		if (rotation.w != 0) {
			R = glm::rotate(rotation.w, vec3(rotation));
		}

		mat4 M = T * R * S;

		return M;
	}

	void renderUI() {
		ImGui::InputFloat3("Translate", glm::value_ptr(translation));
		ImGui::SliderFloat3("Translate sliders", glm::value_ptr(translation), -10.0f, 10.0f);
		ImGui::Text("Rotation");
		ImGui::InputFloat3("Axis", glm::value_ptr(rotation));
		ImGui::SliderFloat("Angle", &rotation.w, 0.0f, glm::two_pi<float>());
		ImGui::InputFloat3("Scale", glm::value_ptr(scale));
	}
};

struct TransformTriangle {
	Transform2D transform = Transform2D();
	Triangle triangle = Triangle();

	TransformTriangle() { }

	TransformTriangle(Transform2D tf, Triangle tri) : transform(tf), triangle(tri) { }

	// TODO: implement this. 
	// Returns a copy of the object's triangle, but the copy has all of its vertices transformed by the 
	// Transform2D component. 
	// That is, for each 
	Triangle getTransformedTri() {
		Triangle newTri = triangle;

		mat4 M = transform.getMatrix();

		for (auto& vertex : newTri.vertices) {
			vec4 position = vec4(vertex.position.x, vertex.position.y, vertex.position.z, 1);
			vec4 newPosition = M * position;
			vertex.position = vec3(newPosition);
		}


		return newTri;
	}

	void renderUI(const char* label = "Triangle") {
		ImGui::PushID((const void*)this);
		if (ImGui::CollapsingHeader(label)) {
			IMDENT;
			ImGui::Text("Transform");
			transform.renderUI();
			ImGui::Text("Triangle");
			triangle.renderUI();
			IMDONT;
		}
		ImGui::PopID();
	}
};

struct TransformIcosphere {
	bool enabled = true;
	bool shouldDelete = false;
	Transform2D transform = Transform2D();

	TransformIcosphere(Transform2D tf) : transform(tf) { }

	void renderUI(const char* label = "Icosphere") {
		ImGui::PushID((const void*)this);
		if (ImGui::CollapsingHeader(label)) {
			IMDENT;
			ImGui::Checkbox("Enabled", &enabled);
			ImGui::Text("Transform");
			transform.renderUI();

			if (ImGui::Button("Delete")) {
				shouldDelete = true;
			}
			IMDONT;
		}
		ImGui::PopID();
	}
};

struct SceneObject {
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

void renderLineParametric(const Line& line, float* pixels, int stride, int width, int height);
void renderLineImplicit(const Line& line, float* pixels, int stride, int width, int height);
void renderCircle(const Circle &circle, float* pixels, int stride, int width, int height);

void renderTriangleOutline(Triangle& tri, float* pixels, int stride, int width, int height);
void renderTriangleParametric(Triangle& tri, float* pixels, int stride, int width, int height);
void renderTriangleBoundingBox(Triangle& tri, float* pixels, int stride, int width, int height);

void renderIcosphere(Icosphere& ico, float* pixels, int stride, int width, int height, bool useColors = true);