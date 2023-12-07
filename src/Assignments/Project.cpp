#include "Project.h"

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

#include "ImGuiFileDialog.h"
#include "OBJMesh.h"
#include "imgui.h"
#include <stb/stb_image.h>

/**
 * ----------------------------------------------------------------------------
 * Cubemap
 * ----------------------------------------------------------------------------
 */
float cubemapVertices[] = {-1.0f, -1.0f, 1.0f,  //        7--------6
                           1.0f,  -1.0f, 1.0f,  //       /|       /|
                           1.0f,  -1.0f, -1.0f, //      4--------5 |
                           -1.0f, -1.0f, -1.0f, //      | |      | |
                           -1.0f, 1.0f,  1.0f,  //      | 3------|-2
                           1.0f,  1.0f,  1.0f,  //      |/       |/
                           1.0f,  1.0f,  -1.0f, //      0--------1
                           -1.0f, 1.0f,  -1.0f};

unsigned int cubemapIndices[] = {
    // Right
    1, 2, 6, 6, 5, 1,
    // Left
    0, 4, 7, 7, 3, 0,
    // Top
    4, 5, 6, 6, 7, 4,
    // Bottom
    0, 3, 2, 2, 1, 0,
    // Back
    0, 1, 5, 5, 4, 0,
    // Front
    3, 7, 6, 6, 2, 3};

std::vector<OBJMesh> meshes;

int activeMeshIndex = 0;
int parallaxLayers = 10;

int windowWidth = 0;
int windowHeight = 0;

float displacementScale = 0.0f;

bool useTexture = false;
bool useNormalTexture = false;
bool validNormalTexture = false;
bool useParallaxTexture = false;
bool useDisplacementMap = false;
bool useWireframe = false;

/**
 * ----------------------------------------------------------------------------
 * Manual flag settings
 * ----------------------------------------------------------------------------
 */

// Enables tessellation for finer details for displacement mapping.
bool useTesselation = false;

// Enables cubemapping but removes all rendered objects and primitives from the
// scene.
bool useCubemapping = false;

// ----------------------------------------------------------------------------

GLuint textureID = 0;
GLuint normalTextureID = 0;
GLuint parallaxTextureID = 0;
GLuint displacementMapID = 0;

vec3 tesselationOuter = vec3(2);
vec3 tesselationInner = vec3(2);

unsigned int cubemapVAO, cubemapVBO, cubemapEBO;
unsigned int cubemapTexture;

#include <stdexcept>

void Project::init() {
  // Load shaders.
  std::string vertexCode = StringUtil::readText("src/shaders/vertex.vert");
  std::string fragmentCode = StringUtil::readText("src/shaders/fragment.frag");
  std::string tessControlCode =
      StringUtil::readText("src/shaders/tessellation.tesc");
  std::string tessEvalCode =
      StringUtil::readText("src/shaders/tessellation.tese");

  std::string cubemapVertexCode =
      StringUtil::readText("src/shaders/cubemap.vert");
  std::string cubemapFragmentCode =
      StringUtil::readText("src/shaders/cubemap.frag");

  const char *ProjectVertexShaderSrc = vertexCode.c_str();
  const char *ProjectFragmentShaderSrc = fragmentCode.c_str();
  const char *tessControlShaderSrc = tessControlCode.c_str();
  const char *tessEvalShaderSrc = tessEvalCode.c_str();

  const char *cubemapVertexShaderSrc = cubemapVertexCode.c_str();
  const char *cubemapFragmentShaderSrc = cubemapFragmentCode.c_str();

  if (useTesselation) {
    if (!shader.init(ProjectVertexShaderSrc, ProjectFragmentShaderSrc,
                     tessControlShaderSrc, tessEvalShaderSrc)) {
      exit(1);
    }
  } else {
    if (!shader.init(ProjectVertexShaderSrc, ProjectFragmentShaderSrc)) {
      exit(1);
    }
  }

  // Default sphere mesh
  OBJMesh sphereMesh = OBJMesh::getSphere(shader.program);
  meshes.push_back(sphereMesh);

  if (useCubemapping) {
    if (!cubemapShader.init(cubemapVertexShaderSrc, cubemapFragmentShaderSrc)) {
      exit(1);
    } else {
      log("Loaded cubemap shader\n");
    }

    glGenVertexArrays(1, &cubemapVAO);
    glGenBuffers(1, &cubemapVBO);
    glGenBuffers(1, &cubemapEBO);
    glBindVertexArray(cubemapVAO);

    GLuint positionAttrib =
        glGetAttribLocation(cubemapShader.program, "position");
    glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubemapVertices), cubemapVertices,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubemapEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubemapIndices),
                 cubemapIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    std::string parentDir =
        (std::filesystem::current_path().std::filesystem::path::parent_path())
            .string();
    parentDir += "/3480-projectv2";

    std::string facesCubemap[6] = {
        parentDir + "/textures/environment_maps/right.jpg",
        parentDir + "/textures/environment_maps/left.jpg",
        parentDir + "/textures/environment_maps/top.jpg",
        parentDir + "/textures/environment_maps/bottom.jpg",
        parentDir + "/textures/environment_maps/front.jpg",
        parentDir + "/textures/environment_maps/back.jpg"};

    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (unsigned int i = 0; i < 6; i++) {
      int width, height, nrChannels;
      unsigned char *data =
          stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);

      if (data) {
        stbi_set_flip_vertically_on_load(false);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width,
                     height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
      } else {
        log("Failed to load cubemap texture: {0}\n", facesCubemap[i]);
        stbi_image_free(data);
        log("Reason: {0}\n", stbi_failure_reason());
      }
    }
  }

  // Guard for when running non cubemap mode to make sure cubemap camera is off.
  // This is because the cubemap camera results in weird rendering effects.
  if (useCubemapping == false) {
    camera.useCubemapCamera = false;
  }

  addSceneObject();
  initialized = true;
}

// Renders to the "screen" texture that has been passed in as a parameter
void Project::render(s_ptr<Framebuffer> framebuffer) {
  if (!initialized) {
    init();
  }

  windowWidth = framebuffer->width;
  windowHeight = framebuffer->height;

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glDisable(GL_CULL_FACE);

  if (useTesselation) {
    useWireframe = true;
  }

  if (useWireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  double deltaTime = Application::get().deltaTime;
  double timeSinceStart = Application::get().timeSinceStart;

  // Same as shader.activate()
  glUseProgram(shader.program);

  if (useCubemapping) {
    glUseProgram(cubemapShader.program);
    glUniform1i(glGetUniformLocation(cubemapShader.program, "cubemap"), 0);
  }

  OBJMesh *activeMesh = &meshes[activeMeshIndex];
  glBindVertexArray(activeMesh->VAO);

  camera.update(float(framebuffer->width), float(framebuffer->height));

  if (light.autoOrbit) {
    vec3 forward = vec3(0, 0, -1);

    if (glm::abs(glm::dot(forward, light.orbitAxis)) > 0.99f) {
      forward = vec3(1, 0, 0);
    }

    mat4 orbit = glm::rotate((float)timeSinceStart, light.orbitAxis);
    light.lightDirection = orbit * vec4(forward, 0);
  }

  mat4 vp = camera.projection * camera.view;
  for (auto &sceneObject : sceneObjects) {
    mat4 model = sceneObject.transform.getMatrixGLM();
    mat4 mvp = vp * model;

    GLuint mvpLocation = glGetUniformLocation(shader.program, "mvp");
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));

    GLuint modelLocation = glGetUniformLocation(shader.program, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

    mat4 normalMatrix = glm::transpose(glm::inverse(model));
    GLuint normalMatrixLocation =
        glGetUniformLocation(shader.program, "normalMatrix");
    glUniformMatrix4fv(normalMatrixLocation, 1, GL_FALSE,
                       glm::value_ptr(normalMatrix));

    // Ambient.
    glUniform1f(glGetUniformLocation(shader.program, "Ia"), light.Ia);
    glUniform3fv(glGetUniformLocation(shader.program, "Ka"), 1,
                 glm::value_ptr(light.Ka));

    // Diffuse.
    glUniform1f(glGetUniformLocation(shader.program, "Id"), light.Id);
    glUniform3fv(glGetUniformLocation(shader.program, "Kd"), 1,
                 glm::value_ptr(light.Kd));

    // Specular.
    glUniform1f(glGetUniformLocation(shader.program, "shininess"),
                light.shininess);
    glUniform3fv(glGetUniformLocation(shader.program, "Ks"), 1,
                 glm::value_ptr(light.Ks));

    glUniform3fv(glGetUniformLocation(shader.program, "lightDirection"), 1,
                 glm::value_ptr(light.lightDirection));

    glUniform3fv(glGetUniformLocation(shader.program, "cameraPosition"), 1,
                 glm::value_ptr(camera.cameraPosition));

    glUniform4fv(glGetUniformLocation(shader.program, "color"), 1,
                 glm::value_ptr(sceneObject.color));

    /**
     * ------------------------------------------------------------------------
     * Textures
     * ------------------------------------------------------------------------
     */
    if (useTexture) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, textureID);

      if (OpenGLRenderer::instance->textures.find(textureID) !=
          OpenGLRenderer::instance->textures.end()) {
        auto texture = OpenGLRenderer::instance->textures[textureID];

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                        texture->wrapS._to_integral());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                        texture->wrapT._to_integral());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        texture->minFilter._to_integral());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        texture->magFilter._to_integral());
      }
    }

    if (useNormalTexture) {
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, normalTextureID);

      if (OpenGLRenderer::instance->textures.find(normalTextureID) !=
          OpenGLRenderer::instance->textures.end()) {
        auto normalTexture =
            OpenGLRenderer::instance->textures[normalTextureID];
        log("Valid normal texture ID: {0}\n", normalTextureID);
        validNormalTexture = true;

        // Set texture parameters.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                        normalTexture->wrapS._to_integral());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                        normalTexture->wrapT._to_integral());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        normalTexture->minFilter._to_integral());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        normalTexture->magFilter._to_integral());

        // Set the texture unit index (e.g., 1) to the normal map uniform.
        glUniform1i(glGetUniformLocation(shader.program, "normalTexture"), 1);
      } else {
        log("Invalid normal texture ID: {0}\n", normalTextureID);
        validNormalTexture = false;
      }
    }

    if (useParallaxTexture) {
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, parallaxTextureID);

      if (OpenGLRenderer::instance->textures.find(parallaxTextureID) !=
          OpenGLRenderer::instance->textures.end()) {
        auto parallaxTexture =
            OpenGLRenderer::instance->textures[parallaxTextureID];

        log("Parallax texture id: {0}\n", parallaxTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                        parallaxTexture->wrapS._to_integral());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                        parallaxTexture->wrapT._to_integral());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        parallaxTexture->minFilter._to_integral());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        parallaxTexture->magFilter._to_integral());

        // Set the texture unit index (e.g., 1) to the normal map uniform.
        glUniform1i(glGetUniformLocation(shader.program, "heightmapTexture"),
                    2);
      }
    }

    if (useDisplacementMap && useParallaxTexture == false) {
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, displacementMapID);

      if (OpenGLRenderer::instance->textures.find(displacementMapID) !=
          OpenGLRenderer::instance->textures.end()) {
        auto displacementMap =
            OpenGLRenderer::instance->textures[displacementMapID];

        log("Displacement map id: {0}\n", displacementMapID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                        displacementMap->wrapS._to_integral());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                        displacementMap->wrapT._to_integral());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        displacementMap->minFilter._to_integral());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        displacementMap->magFilter._to_integral());

        // Set the texture unit index (e.g., 1) to the normal map uniform.
        glUniform1i(glGetUniformLocation(shader.program, "displacementMap"), 2);
      }
    }

    glUniform1i(glGetUniformLocation(shader.program, "useTexture"),
                (GLint)useTexture);

    // Checker for valid normal texture id.
    glUniform1i(glGetUniformLocation(shader.program, "validNormalTexture"),
                (GLint)validNormalTexture);

    glUniform1i(glGetUniformLocation(shader.program, "useNormalTexture"),
                (GLint)useNormalTexture);

    glUniform1i(glGetUniformLocation(shader.program, "useParallaxTexture"),
                (GLint)useParallaxTexture);

    glUniform1i(glGetUniformLocation(shader.program, "parallaxLayers"),
                parallaxLayers);

    glUniform1i(glGetUniformLocation(shader.program, "useDisplacementMap"),
                (GLint)useDisplacementMap);

    glUniform1f(glGetUniformLocation(shader.program, "displacementScale"),
                displacementScale);

    glUniform3fv(glGetUniformLocation(shader.program, "outerTesselation"), 1,
                 glm::value_ptr(tesselationOuter));

    glUniform3fv(glGetUniformLocation(shader.program, "innerTesselation"), 1,
                 glm::value_ptr(tesselationInner));

    if (useTesselation) {
      // Set tessellation levels
      float outerTessLevels[] = {2.0, 2.0, 2.0};
      float innerTessLevels[] = {2.0};
      glPatchParameteri(GL_PATCH_VERTICES, 3);
      glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, outerTessLevels);
      glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, innerTessLevels);
      glDrawElements(GL_PATCHES, activeMesh->indices.size(), GL_UNSIGNED_INT,
                     0);
    } else {
      glDrawElements(GL_TRIANGLES, activeMesh->indices.size(), GL_UNSIGNED_INT,
                     0);
    }
  }

  if (useCubemapping) {
    glDepthFunc(GL_LEQUAL);

    glUseProgram(cubemapShader.program);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::vec3 cameraOrientation = glm::vec3(0.0f, 0.0f, -1.0f);

    // Rotate according to camera orientation.
    view = glm::mat4(glm::mat3(glm::lookAt(
        camera.cameraPosition, camera.cameraPosition + camera.cameraOrientation,
        camera.cameraUp)));

    projection = glm::perspective(
        glm::radians(45.0f), (float)windowWidth / windowHeight, 0.1f, 100.0f);

    GLuint viewLocation = glGetUniformLocation(cubemapShader.program, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(
        glGetUniformLocation(cubemapShader.program, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));

    glBindVertexArray(cubemapVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
}

/**
 * ----------------------------------------------------------------------------
 * UI Controls
 * ----------------------------------------------------------------------------
 */
void Project::renderUI() {
  if (useCubemapping == false) {
    ImGui::Checkbox("Use texture", &useTexture);
    if (useTexture) {
      int texID = textureID;
      if (ImGui::InputInt("Texture ID", &texID)) {
        textureID = texID;
      }
    }

    ImGui::Checkbox("Use normal texture", &useNormalTexture);
    if (useNormalTexture) {
      int texID = normalTextureID;
      if (ImGui::InputInt("Normal Texture ID", &texID)) {
        normalTextureID = texID;
      }
    }

    if (useTesselation == false && useDisplacementMap == false) {
      ImGui::Checkbox("Use parallax texture", &useParallaxTexture);
      if (useParallaxTexture) {
        int texID = parallaxTextureID;
        if (ImGui::InputInt("Parallax Texture ID", &texID)) {
          parallaxTextureID = texID;
        }
        ImGui::SliderInt("Parallax layers", &parallaxLayers, 0, 100);
      }
    }

    if (useParallaxTexture == false) {
      ImGui::Checkbox("Use displacement map", &useDisplacementMap);
      if (useDisplacementMap) {
        int texID = displacementMapID;
        if (ImGui::InputInt("Displacement Map ID", &texID)) {
          displacementMapID = texID;
        }
        ImGui::SliderFloat("Displacement scale", &displacementScale, 0.0f,
                           1.0f);
      }
    }

    if (useTesselation) {
      ImGui::Checkbox("Enable Tesselation", &useTesselation);
      int tesselationLevelOuter = tesselationOuter.x;
      int tesselationLevelInner = tesselationInner.x;
      if (useTesselation) {
        if (ImGui::SliderInt("Tesselation level outer", &tesselationLevelOuter,
                             2, 100)) {
          tesselationOuter = vec3(tesselationLevelOuter);
        }
        if (ImGui::SliderInt("Tesselation level inner", &tesselationLevelInner,
                             2, 100)) {
          tesselationInner = vec3(tesselationLevelInner);
        }
      }
    }

    ImGui::Checkbox("Wireframe", &useWireframe);
  }

  int numMeshes = meshes.size();
  ImGui::Text("Number of meshes: %d", numMeshes);

  int meshMax = meshes.size() - 1;
  if (meshMax > 0) {
    ImGui::SliderInt("Active mesh index", &activeMeshIndex, 0, meshMax);
  }

  if (ImGui::Button("Load model")) {
    ImGuiFileDialog::Instance()->OpenDialog("ChooseOBJKey", "Choose OBJ",
                                            ".obj", ".");
  }

  light.renderUI();
  camera.renderUI();

  if (ImGuiFileDialog::Instance()->Display("ChooseOBJKey")) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      std::string objFile = ImGuiFileDialog::Instance()->GetFilePathName();
      OBJMesh loadedMesh = OBJMesh::import(objFile.c_str(), shader.program);
      if (loadedMesh.isValid()) {
        meshes.push_back(loadedMesh);
      }

      ImGuiFileDialog::Instance()->Close();
    }
  }

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
    static vec2 scaleRange = vec2(0.1, 8.f);
    if (ImGui::Button(addRandomLabel.c_str())) {
      for (int i = 0; i < glm::max(numberToAdd, 0); i++) {
        SceneObject newObject;
        newObject.name = fmt::format("Object {0}", sceneObjects.size() + 1);

        newObject.transform.translation =
            glm::linearRand(vec3(-200), vec3(200));
        newObject.transform.rotation =
            vec4(glm::sphericalRand(1.0f),
                 glm::linearRand(0.f, glm::two_pi<float>()));
        newObject.transform.scale =
            vec3(glm::linearRand(scaleRange.x, scaleRange.y));
        newObject.color = glm::linearRand(vec3(0.1f), vec3(1.f));

        sceneObjects.push_back(newObject);
      }
    }

    ImGui::InputFloat2("Scale min/max", glm::value_ptr(scaleRange));

    if (sceneObjects.size() > 0) {
      if (ImGui::Button("Clear all")) {
        sceneObjects.clear();
      }

      int counter = 1;

      ImGui::Text("Number of objects: %lu", sceneObjects.size());

      auto soToDelete = sceneObjects.end();
      for (auto it = sceneObjects.begin();
           it != sceneObjects.end() && counter++ < 100; ++it) {
        auto &cb = *it;
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
