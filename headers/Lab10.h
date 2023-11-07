#pragma once

#include "Assignment.h"

#include "GPU.h"

MAKE_ENUM(RTShapeType, int, Plane, Tri, Quad, Sphere, Box)

struct Ray {
	vec3 p0;
	vec3 R;

	vec3 p(float t) const {
		return p0 + t * R;
	}
};

struct RaySceneObject;

struct HitRecord {
	bool hit = false;
	float t = std::numeric_limits<float>::max();
	vec3 position;
	vec3 normal;
	const RaySceneObject* object = nullptr;
};

struct Material {
	float reflective = 0.0f;
	vec3 Kd = vec3(1.f);

	void renderUI() {
		ImGui::SliderFloat("Reflectivity", &reflective, 0, 1);
		ImGui::ColorEdit3("Diffuse", glm::value_ptr(Kd));
	}
};

struct RaySceneObject {
	bool enabled = true;
	bool changed = true;
	RTShapeType type = RTShapeType::Plane;
	std::string name = "object";
	Material material;
	Transform2D transform;

	// Cached values to speed up raytracing
	mat4 R;		// Rotation matrix
	mat4 invR;	// Inverse rotation matrix

	// For planes: xyz is normal, w is d
	// For spheres: xyz is center, w is radius
	vec4 data;

	virtual void hit(const Ray& ray, HitRecord& hr);

	virtual void renderUI();
};

class RayTracer : public Assignment {
public:

	Camera camera;
	Lighting light;

	s_ptr_vector<RaySceneObject> sceneObjects;

	RayTracer() : Assignment("Ray tracer", false) { }
	virtual ~RayTracer() { }

	virtual void init();
	virtual void render(s_ptr<Texture> screen);

	s_ptr<RaySceneObject> addSceneObject() {
		s_ptr<RaySceneObject> newObject = std::make_shared<RaySceneObject>();
		newObject->name = fmt::format("Object {0}", sceneObjects.size() + 1);
		sceneObjects.push_back(newObject);
		return newObject;
	}

	s_ptr<RaySceneObject> addPlane(const vec3& position = vec3(0.f), const vec3& normal = vec3(0.f, 1.f, 0.f), const vec2 & scale = vec2(0.f)) {
		s_ptr<RaySceneObject> newObject = addSceneObject();
		newObject->type = RTShapeType::Plane;
		newObject->transform.translation = position;
		newObject->transform.rotation = vec4(glm::degrees(glm::eulerAngles(glm::rotation(vec3(0, 1, 0), normal))), 0.f);

		if (scale.x > 0.f && scale.y > 0.f) {
			newObject->transform.scale = vec3(scale.x, 0.f, scale.y);
		}
		else {
			newObject->transform.scale = vec3(0.f);
		}
		return newObject;
	}

	s_ptr<RaySceneObject> addSphere(const vec3& position = vec3(0.f), float radius = 1.0f) {
		s_ptr<RaySceneObject> newObject = addSceneObject();
		newObject->type = RTShapeType::Sphere;
		newObject->transform.translation = position;
		newObject->data.w = radius;
		return newObject;
	}

	virtual void renderUI();
};
