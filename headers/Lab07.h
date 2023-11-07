#pragma once

#include "Assignment.h"

#include "GPU.h"

class Lab07 : public Assignment {
public:
	TriangleMeshSOA cubeSOA;
	TriangleMeshAOS cubeAOS;
	IndexBuffer indexBuffer;
	Shader shader;

	Camera camera;
	std::vector<SceneObject> sceneObjects;

	bool renderSOA = true;

	Lab07() : Assignment("Lab 07", true) { }
	virtual ~Lab07() { }

	virtual void init();
	virtual void render(s_ptr<Framebuffer> framebuffer);
	virtual void renderUI();

	void addSceneObject() {
		SceneObject newObject;
		newObject.name = fmt::format("Object {0}", sceneObjects.size() + 1);
		sceneObjects.push_back(newObject);
	}
};
