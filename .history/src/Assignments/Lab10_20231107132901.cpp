#include "Lab10.h"

#include "Application.h"

#include "Texture.h"

#include "UIHelpers.h"

MAKE_ENUM(RTColorMode, int, Default, Position, Normal);

RTColorMode colorMode = RTColorMode::Default;

void RaySceneObject::renderUI() {
	ImGui::PushID((const void*)this);

	if (ImGui::CollapsingHeader(name.c_str())) {

		changed |= ImGui::Checkbox("Enabled", &enabled);
		changed |= renderEnumDropDown("Shape type", type);
		if (ImGui::CollapsingHeader("Transform")) {
			changed |= ImGui::InputFloat3("Translate", glm::value_ptr(transform.translation));
			changed |= ImGui::SliderFloat3("Translate sliders", glm::value_ptr(transform.translation), -10.0f, 10.0f);

			if (type == +RTShapeType::Plane) {
				ImGui::Text("Rotation");
				changed |= ImGui::SliderFloat3("Euler angles", glm::value_ptr(transform.rotation), 0.0f, 360.0f);
				changed |= ImGui::InputFloat3("Scale", glm::value_ptr(transform.scale));
			}
			else if (type == +RTShapeType::Sphere) {
				changed |= ImGui::InputFloat("Radius", &data.w);
			}
			
		}
		if (ImGui::CollapsingHeader("Material")) {
			material.renderUI();
		}
	}
	
	ImGui::PopID();
}

// Compute whether a ray hits a plane:
// ray p(t) = p0 + r*t
// plane p = ax + by + cz + d = 0
//		p*n + d = 0
//		(p0 + rt) . n + d = 0
//		p0 . n + tr . n + d = 0
//		tr . n = -(d + p0.n)
//		t = -(d + p0 . n) / (r . n)
void RayPlaneHit(const RaySceneObject& object, const Ray& ray, HitRecord& hr) {

	// Compute the plane's normal by transforming the unit up vector <0, 1, 0>
	vec3 N = object.R * vec4(0, 1, 0, 0);

	// Compute the plane's d value by plugging in the plane's center into the plane equation
	// p . n + d = 0
	// d = - (p . n)
	float d = -glm::dot(object.transform.translation, N);

	float t = -(d + glm::dot(ray.p0, N)) / glm::dot(ray.R, N);

  // TODO: implement ray-plane intersection test
	if (t > 0 && t < hr.t) {
		hr.hit = true;
		hr.t = t;
		hr.object = &object;
		hr.position = ray.p0 + t * ray.R;
		hr.normal = N;
	}
}

// Compute whether a ray hits a sphere:
// ray p(t) = p0 + r*t
// sphere (P - C) . (P - C) - r^2 = 0
// Substitute p(t) for P in sphere equation:
// (p0 + tR - C) . (p0 + tR - C) - r^2 = 0
// (tR + p0 - C) . (tR + p0 - C) - r^2 = 0
// t^2 R . R + 2 * tR . (p0 - C) + (p0 - C) . (p0 - C) - r^2 = 0
//
// As a quadratic equation:
// at^2 + bt + c = 0, where
// a = R . R
// b = 2 * R . (p0 - C)
// c = (p0 - C) . (p0 - C) - r ^ 2
//
//Solve for t using the quadratic formula :
//t = (-b + / -sqrt(b ^ 2 - 4 * a * c)) / (2 * a)
void RaySphereHit(const RaySceneObject& object, const Ray& ray, HitRecord& hr) {

	vec3 C = object.transform.translation;
	float r = object.data.w;

	if (r <= 0) return;

	float a = glm::dot(ray.R, ray.R);
	float b = 2 * glm::dot(ray.R, ray.p0 - C);
	float c = glm::dot(ray.p0 - C, ray.p0 - C) - r * r;

	float t = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);

  // TODO: implement ray-sphere intersection
  if (t > 0 && t < hr.t) {
		hr.hit = true;
		hr.t = t;
		hr.object = &object;
		hr.position = ray.p0 + t * ray.R;
		hr.normal = glm::normalize(hr.position - C);
	}
}

void RaySceneObject::hit(const Ray& ray, HitRecord& hr) {
	switch (type) {
		case RTShapeType::Plane:
			RayPlaneHit(*this, ray, hr);
			break;
		case RTShapeType::Sphere:
			RaySphereHit(*this, ray, hr);
			break;
		default:
			break;
	}
}

void RayTracer::init() {

	auto floor = addPlane(vec3(0.f, -2.0f, 0.f), vec3(0, 1, 0), vec2(1.f, 1.f));
	floor->material.Kd = vec3(0.6f);

	auto ceil = addPlane(vec3(0.f, 2.0f, 0.f), vec3(0, -1, 0), vec2(1.f, 1.f));
	ceil->material.Kd = vec3(0.6f);

	auto left = addPlane(vec3(-2.0f, 0.f, 0.f), vec3(1, 0, 0), vec2(1.f));
	left->material.Kd = vec3(1.0f, 0.2f, 0.2f);

	auto right = addPlane(vec3(2.0f, 0.f, 0.f), vec3(-1, 0, 0), vec2(1.f));
	right->material.Kd = vec3(1.0f, 0.2f, 0.2f);

	auto back = addPlane(vec3(0.0f, 0.f, -2.f), vec3(0, 0, 1), vec2(1.f));
	back->material.Kd = vec3(0.1f, 0.2f, 0.8f);

	auto sphere = addSphere(vec3(0, -1, 0));
	sphere->material.Kd = vec3(0.f, 1.f, 1.f);

	light.Ia = 0.15f;
	
	initialized = true;
}

void raytraceScene(s_ptr_vector<RaySceneObject> sceneObjects, const Ray& ray, HitRecord& hr) {

	for (auto& o : sceneObjects) {
		if (o->enabled) {
			o->hit(ray, hr);
		}
	}
}

// Renders objects using the framebuffer
void RayTracer::render(s_ptr<Texture> screen) {
	if (!initialized) init();

	float width = screen->resolution.x;
	float height = screen->resolution.y;
	auto mem = screen->memory;
	auto pixels = (float*)mem->value;

	double deltaTime = Application::get().deltaTime;
	double timeSinceStart = Application::get().timeSinceStart;

	int stride = mem->stride;

	size_t maxSize = width * height * stride;

	camera.update(width, height);

	if (light.autoOrbit) {
		vec3 forward = vec3(0, 0, -1);

		if (glm::abs(glm::dot(forward, light.orbitAxis)) > 0.99f) {
			forward = vec3(1, 0, 0);
		}

		mat4 orbit = glm::rotate((float)timeSinceStart, light.orbitAxis);

		light.lightDirection = orbit * vec4(forward, 0);
	}

	// Update rotation matrices for planes so the normal computation in RayPlaneHit is faster
	for (auto& o : sceneObjects) {
		if (o->type == +RTShapeType::Plane && o->changed) {
			// Get a rotation matrix for the plane from its euler angles
			o->R = glm::rotate(glm::radians(o->transform.rotation.z), vec3(0, 0, 1))
				* glm::rotate(glm::radians(o->transform.rotation.y), vec3(0, 1, 0))
				* glm::rotate(glm::radians(o->transform.rotation.x), vec3(1, 0, 0));
			o->changed = false;
		}
	}

	mat4 projection = camera.projection;
	mat4 view = camera.view;

	mat4 invProjection = glm::inverse(projection);
	mat4 invView = glm::inverse(view);

	//#pragma omp parallel for
	for (int x = 0; x < (int)width; x++) {
		for (int y = 0; y < height; y++) {

			// Step 1: convert from screen-space to NDC
			vec2 ss(x, y);
			ss.x = ss.x / (width - 1);
			ss.y = ss.y / (height - 1);
			// ss is now in range of [0, 0] to [1, 1]
			vec2 ndc = (ss * 2.0f) - vec2(1);

			vec3 preview = invProjection * vec4(ndc.x, ndc.y, 0, 1);
			vec3 world = invView * vec4(preview.x, preview.y, preview.z, 1);

			Ray ray = { camera.cameraPosition, glm::normalize(world - camera.cameraPosition) };

			size_t offset = ((y * width) + x) * stride;

			vec4* pixel = (vec4*)(pixels + offset);

			vec4 color;

			HitRecord hr;

			raytraceScene(sceneObjects, ray, hr);
			
			if (hr.hit) {
				if (colorMode == +RTColorMode::Default) {
					color = vec4(hr.object->material.Kd, 1.0f);

					// TODO: use the intersection result data to compute Phong or Blinn-Phong shading.
					vec3 N = hr.normal;

					// Light direction
					vec3 L = glm::normalize(light.lightDirection);

					// View vector
					vec3 V = glm::normalize(camera.cameraPosition - hr.position);

					// Halfway vector
					vec3 H = glm::normalize(L + V);
					float diffuse = glm::max(glm::dot(N, L), 0.0f);
					vec3 diffuseColor = hr.object->material.Kd * light.Id * diffuse * color;
					
					float specular = glm::pow(glm::max(glm::dot(N, H), 0.0f), hr.object->material.reflective);
				}
				// These can be useful for debugging - they set the pixel color to the position or the normal
				// seen at the intersection point. 
				else if (colorMode == +RTColorMode::Position) {
					color = vec4(hr.position, 1.0f);
				}
				else if (colorMode == +RTColorMode::Normal) {
					vec3 n = (hr.normal + vec3(1.f)) * 0.5f;
					color = vec4(n, 1.0f);
				}
				
			}
			else {
				color = vec4(ray.R, 1.0f);
			}
			
			*pixel = color;
		}
	}


	glBindTexture(GL_TEXTURE_2D, screen->id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen->resolution.x, screen->resolution.y, 0, GL_RGBA, GL_FLOAT, (const void*)pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RayTracer::renderUI() {
	ImGui::Text("Coloring mode");
	ImGui::SameLine();
	renderEnumButton(colorMode);
	camera.renderUI();
	light.renderUI();

	if (ImGui::CollapsingHeader("Objects")) {
		if (ImGui::Button("Add object")) {
			addSceneObject();
		}
		for (auto& o : sceneObjects) {
			o->renderUI();
		}
	}
}