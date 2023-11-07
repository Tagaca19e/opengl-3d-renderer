#include "Primitives.h"

Sphere Sphere::instance;

void renderLineParametric(const Line& line, float* pixels, int stride, int width, int height) {
	if (!line.enabled) return;

	vec2 ray = line.p1.position - line.p0.position;
	float rayLength = glm::length(ray);

	float dt = 1.0f / glm::max(1.0f, rayLength - 1.0f);

	size_t maxSize = width * height * stride;

	for (float t = 0.0f; t <= 1.0f; t += dt) {
		vec2 p = vec2(line.p0.position) + t * ray;
		// or
		//vec2 p = (1.0f - t) * line.p0.position + t * line.p1.position;

		int x = glm::floor(p.x);
		int y = glm::floor(p.y);
		size_t offset = ((y * width) + x) * stride;
		if (offset >= maxSize) break;

		auto pixel = (vec3*)(pixels + offset);

		*pixel = line.color * glm::mix(line.p0.color, line.p1.color, t);
	}

}

void renderLineImplicit(const Line& line, float* pixels, int stride, int width, int height) {
	if (!line.enabled) return;

	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {

			size_t offset = ((y * width) + x) * stride;

			auto pixel = (vec3*)(pixels + offset);

			float t = 0.0f;
			// Distance from the current pixel's point to the line
			float dist = line.dist(vec2(x, y) + vec2(0.5f), t);

			if (dist <= line.thickness * 0.5f) {
				float dt = dist / (line.thickness * 0.5f);
				vec3 resultColor = line.color * glm::clamp(1.0f - dt * dt, 0.0f, 1.0f);
				resultColor *= glm::mix(line.p0.color, line.p1.color, glm::clamp(t, 0.f, 1.f));
				*pixel = resultColor;
			}
		}
	}
}

void renderCircle(const Circle& circle, float* pixels, int stride, int width, int height) {
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {

			size_t offset = ((y * width) + x) * stride;

			auto pixel = (vec3*)(pixels + offset);

			// Signed distance: could be 0 (on the circle), positive (outside), negative (inside)
			float dist = circle.dist(vec2(x, y));

			if (dist <= circle.radius) {
				*pixel = circle.center.color;
			}
		}
	}
}

// Draws outline of triangle using parametric lines
void renderTriangleOutline(Triangle& tri, float* pixels, int stride, int width, int height) {
	for (int i = 0; i < 3; i++) {
		Line newLine;
		newLine.p0 = tri.vertices[i];
		newLine.p1 = tri.vertices[(i + 1) % 3];
		newLine.color = tri.color;
		renderLineParametric(newLine, pixels, stride, width, height);
	}
}

// Draw filled triangle by incrementing through the barycentric coordinates
void renderTriangleParametric(Triangle& tri, float* pixels, int stride, int width, int height) {
	vec3 r1 = tri.vertices[1].position - tri.vertices[0].position;
	vec3 r2 = tri.vertices[2].position - tri.vertices[0].position;

	float l1 = glm::length(r1);
	float l2 = glm::length(r2);

	float dt1 = 1.0f / glm::max(1.0f, 1.25f * l1);
	float dt2 = 1.0f / glm::max(1.0f, 1.25f * l2);

	size_t maxSize = width * height * stride;

	for (float t1 = 0.0f; t1 <= 1.0f; t1 += dt1) {
		for (float t2 = 0.0f; t2 <= 1.0f; t2 += dt2) {
			if (t1 + t2 >= 1.0f) continue;
			float t0 = 1.0f - t1 - t2;

			Vertex interpolated = tri.computeFromBarycentric(vec3(t0, t1, t2));
			int x = glm::floor(interpolated.position.x);
			int y = glm::floor(interpolated.position.y);

			size_t offset = ((y * width) + x) * stride;

			if (offset < maxSize - 3) {
				vec4* pixel = (vec4*)(pixels + offset);
				if (interpolated.position.z < pixel->w) {
					*pixel = vec4(tri.color * interpolated.color, interpolated.position.z);
				}
			}

		}
	}
}

// Draw filled triangle by traversing the bounding box around the triangle 
// and checking each pixel within to see if it's inside the triangle
void renderTriangleBoundingBox(Triangle& tri, float* pixels, int stride, int width, int height) {
	vec2 min(width, height);
	vec2 max(0, 0);

	for (int i = 0; i < 3; i++) {
		min = glm::min(min, vec2(tri.vertices[i].position));
		max = glm::max(max, vec2(tri.vertices[i].position));
	}

	min = glm::clamp(min, vec2(0.f), vec2(width - 1, height - 1));
	max = glm::clamp(max, vec2(0.f), vec2(width - 1, height - 1));

	size_t maxSize = width * height * stride;


	for (int y = min.y; y <= max.y; y++) {
		for (int x = min.x; x <= max.x; x++) {
			vec3 bary = tri.barycentric(vec3(x, y, 0));

			if (tri.baryInTriangle(bary)) {
				size_t offset = ((y * width) + x) * stride;

				if (offset < maxSize - 3) {
					vec4* pixel = (vec4*)(pixels + offset);

					Vertex interpolated = tri.computeFromBarycentric(bary);

					if (interpolated.position.z < pixel->w) {
						*pixel = vec4(tri.color * interpolated.color, interpolated.position.z);
					}
				}
			}
		}
	}
}

void renderIcosphere(Icosphere& ico, float* pixels, int stride, int width, int height, bool useColors) {
	for (auto& triangleIndices : ico.indices) {
		Triangle tri;

		for (int i = 0; i < 3; i++) {
			vec3 p = ico.positions[triangleIndices[i]];
			tri.vertices[i].position = p;
			tri.vertices[i].color = vec3(1.0f - p.z);
			if (useColors) {
				tri.vertices[i].color *= ico.colors[triangleIndices[i]];
			}
		}	

		renderTriangleBoundingBox(tri, pixels, stride, width, height);
	}
}