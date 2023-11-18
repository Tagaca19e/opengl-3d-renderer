#pragma once

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
#include "imgui.h"

struct OBJMeshVertex {
  vec3 position;
  vec3 normal;
  vec2 texCoord;
  vec3 tangent;
  vec3 bitangent;
};

struct OBJMesh {
  std::vector<OBJMeshVertex> vertices;
  std::vector<GLuint> indices;

  GLuint VAO = 0;
  GLuint VBO = 0;
  GLuint IBO = 0;

  bool isValid() {
    return VAO != 0 && VBO != 0 && IBO != 0 && !vertices.empty() &&
           !indices.empty();
  }

  static OBJMesh import(const char *objFile, GLuint shaderProgram) {
    OBJMesh mesh;

    std::ifstream fin(objFile);

    if (!fin) {
      log("Error creating input stream on {0}\n", objFile);
      return mesh;
    }

    std::string line;

    std::vector<vec3> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> texCoords;

    GLuint faceIndex = 0;

    while (std::getline(fin, line)) {
      auto lineEntries = StringUtil::split(line, ' ');

      if (lineEntries.empty()) {
        continue;
      }

      if (lineEntries[0] == "v" || lineEntries[0] == "vn") {
        vec3 v;
        v[0] = std::atof(lineEntries[1].c_str());
        v[1] = std::atof(lineEntries[2].c_str());
        v[2] = std::atof(lineEntries[3].c_str());

        if (lineEntries[0] == "v") {
          vertices.push_back(v);
        } else if (lineEntries[0] == "vn") {
          normals.push_back(v);
        }
      } else if (lineEntries[0] == "vt") {
        vec2 uv;
        uv[0] = std::atof(lineEntries[1].c_str());
        uv[1] = std::atof(lineEntries[2].c_str());

        texCoords.push_back(uv);
      } else if (lineEntries[0] == "f") {
        if (StringUtil::contains(line, "/")) {
          for (int i = 1; i < lineEntries.size(); i++) {
            int positionIndex = -1, normalIndex = -1, texCoordIndex = -1;

            auto faceComponents = StringUtil::split(lineEntries[i], '/', false);

            if (faceComponents.size() == 2) {
              positionIndex = std::stoi(faceComponents[0]) - 1;
              texCoordIndex = std::stoi(faceComponents[1]) - 1;
            } else if (faceComponents.size() == 3) {
              positionIndex = std::stoi(faceComponents[0]) - 1;
              if (faceComponents[1] != "") {
                texCoordIndex = std::stoi(faceComponents[1]) - 1;
              }
              normalIndex = std::stoi(faceComponents[2]) - 1;
            }

            OBJMeshVertex omv;
            omv.position = vertices[positionIndex];
            if (normals.empty()) {
              omv.normal = glm::normalize(omv.position);
            } else {
              omv.normal = normals[normalIndex];
            }

            if (texCoordIndex > -1) {
              omv.texCoord = texCoords[texCoordIndex];
            }

            mesh.vertices.push_back(omv);
            mesh.indices.push_back(faceIndex++);
          }

          // 2 tris per face (quad)
          if (lineEntries.size() == 5) {
            // Turn (0, 1, 2, 3) into two separate tris: (0, 1, 2) and (2, 3,
            // 0). (0, 1, 2) is already in the mesh. Create (2, 3, 0);

            // Get vertex 3 which has already been pushed on.
            auto v3 = mesh.vertices.back();
            mesh.vertices.pop_back();

            // Get vertex 2 which has to be copied
            auto v2 = mesh.vertices.back();

            // Get vertex 0 which is 2 behind this one
            auto v0 = mesh.vertices[mesh.vertices.size() - 3];
            mesh.vertices.push_back(v2);
            mesh.vertices.push_back(v3);
            mesh.vertices.push_back(v0);
            mesh.indices.push_back(faceIndex++);
            mesh.indices.push_back(faceIndex++);
          }

          // Compute tangent and bitangent vectors from the last 3 entries.
          size_t nv = mesh.vertices.size();
          vec3 pos1 = mesh.vertices[nv - 3].position;
          vec3 pos2 = mesh.vertices[nv - 2].position;
          vec3 pos3 = mesh.vertices[nv - 1].position;
          vec2 uv1 = mesh.vertices[nv - 3].texCoord;
          vec2 uv2 = mesh.vertices[nv - 2].texCoord;
          vec2 uv3 = mesh.vertices[nv - 1].texCoord;

          vec3 edge1 = pos2 - pos1;
          vec3 edge2 = pos3 - pos1;
          vec2 deltaUV1 = uv2 - uv1;
          vec2 deltaUV2 = uv3 - uv1;

          float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
          vec3 tangent, bitangent;

          tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
          tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
          tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

          bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
          bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
          bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

          for (int i = 3; i > 0; i--) {
            mesh.vertices[nv - i].tangent = tangent;
            mesh.vertices[nv - i].bitangent = bitangent;
          }
        } else {
          for (int i = 1; i < lineEntries.size(); i++) {

            int positionIndex = std::stoi(lineEntries[i]) - 1;

            OBJMeshVertex omv;
            omv.position = vertices[positionIndex];
            omv.normal = vec3(0.f);

            mesh.vertices.push_back(omv);
            mesh.indices.push_back(faceIndex++);
          }
        }
      }
    }

    glGenVertexArrays(1, &mesh.VAO);
    glBindVertexArray(mesh.VAO);

    glGenBuffers(1, &mesh.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(OBJMeshVertex) * mesh.vertices.size(),
                 (const void *)mesh.vertices.data(), GL_STATIC_DRAW);

    GLint positionLocation = glGetAttribLocation(shaderProgram, "vPosition");
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE,
                          sizeof(OBJMeshVertex),
                          (const void *)offsetof(OBJMeshVertex, position));
    glEnableVertexAttribArray(positionLocation);

    GLint normalLocation = glGetAttribLocation(shaderProgram, "vNormal");
    glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE,
                          sizeof(OBJMeshVertex),
                          (const void *)offsetof(OBJMeshVertex, normal));
    glEnableVertexAttribArray(normalLocation);

    GLint texCoordLocation = glGetAttribLocation(shaderProgram, "texCoord");
    glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE,
                          sizeof(OBJMeshVertex),
                          (const void *)offsetof(OBJMeshVertex, texCoord));
    glEnableVertexAttribArray(texCoordLocation);

    GLint tanLocation = glGetAttribLocation(shaderProgram, "vTangent");
    if (tanLocation > -1) {
      glVertexAttribPointer(tanLocation, 3, GL_FLOAT, GL_FALSE,
                            sizeof(OBJMeshVertex),
                            (const void *)offsetof(OBJMeshVertex, tangent));
      glEnableVertexAttribArray(tanLocation);
    }

    GLint bitanLocation = glGetAttribLocation(shaderProgram, "vBitangent");
    if (bitanLocation > -1) {
      glVertexAttribPointer(bitanLocation, 3, GL_FLOAT, GL_FALSE,
                            sizeof(OBJMeshVertex),
                            (const void *)offsetof(OBJMeshVertex, bitangent));
      glEnableVertexAttribArray(bitanLocation);
    }

    glGenBuffers(1, &mesh.IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.indices.size(),
                 (const void *)mesh.indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    return mesh;
  }

  static OBJMesh getSphere(GLuint shaderProgram) {
    OBJMesh sphere;
    glGenVertexArrays(1, &sphere.VAO);
    glBindVertexArray(sphere.VAO);

    glGenBuffers(1, &sphere.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, sphere.VBO);

    for (auto &vertex : Sphere::instance.positions) {
      OBJMeshVertex omv;
      omv.position = vertex;
      omv.normal = glm::normalize(omv.position);
      sphere.vertices.push_back(omv);
    }

    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(OBJMeshVertex) * sphere.vertices.size(),
                 (const void *)sphere.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &sphere.IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere.IBO);
    for (auto &tri : Sphere::instance.indices) {
      for (int i = 0; i < 3; i++) {
        sphere.indices.push_back(tri[i]);
      }
    }

    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(GLuint) * sphere.indices.size(),
                 (const void *)sphere.indices.data(), GL_STATIC_DRAW);

    GLint positionLocation = glGetAttribLocation(shaderProgram, "vPosition");
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE,
                          sizeof(OBJMeshVertex),
                          (const void *)offsetof(OBJMeshVertex, position));
    glEnableVertexAttribArray(positionLocation);

    GLint normalLocation = glGetAttribLocation(shaderProgram, "vNormal");
    glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE,
                          sizeof(OBJMeshVertex),
                          (const void *)offsetof(OBJMeshVertex, normal));
    glEnableVertexAttribArray(normalLocation);

    glBindVertexArray(0);

    return sphere;
  }
};
