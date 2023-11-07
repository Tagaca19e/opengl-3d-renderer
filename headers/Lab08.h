#pragma once

#include "Assignment.h"

#include "GPU.h"

class Lab08 : public Assignment {
public:
	Shader shader;

	Camera camera;
	Lighting light;

	std::vector<SceneObject> sceneObjects;

	Lab08() : Assignment("Lab 08", true) { }
	virtual ~Lab08() { }

	virtual void init();
	virtual void render(s_ptr<Framebuffer> framebuffer);
	virtual void renderUI();

	void addSceneObject() {
		SceneObject newObject;
		newObject.name = fmt::format("Object {0}", sceneObjects.size() + 1);
		sceneObjects.push_back(newObject);
	}
};
