#pragma once

#include "Assignment.h"

#include "GPU.h"

class Project : public Assignment {
public:
  Shader shader;
  Shader cubemapShader;

  Camera camera;
  Lighting light;

  std::vector<SceneObject> sceneObjects;

  Project() : Assignment("Project", true) {}
  virtual ~Project() {}

  virtual void init();
  virtual void render(s_ptr<Framebuffer> framebuffer);
  virtual void renderUI();

  void addSceneObject() {
    SceneObject newObject;
    newObject.name = fmt::format("Object {0}", sceneObjects.size() + 1);
    log("new object name: {0}\n", newObject.name);
    sceneObjects.push_back(newObject);
  }
};
