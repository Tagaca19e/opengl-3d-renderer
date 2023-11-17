#include "Lab08.h"

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
      // log("{0}\n", line);

      auto lineEntries = StringUtil::split(line, ' ');

      if (lineEntries.empty())
        continue;

      if (lineEntries[0] == "v" || lineEntries[0] == "vn") {
        vec3 v;
        v[0] = std::atof(lineEntries[1].c_str());
        v[1] = std::atof(lineEntries[2].c_str());
        v[2] = std::atof(lineEntries[3].c_str());

        if (lineEntries[0] == "v") {
          // log("Vertex {0}: {1}\n", vertices.size(), glm::to_string(v));
          vertices.push_back(v);
        } else if (lineEntries[0] == "vn") {
          // log("Normal {0}: {1}\n", normals.size(), glm::to_string(v));
          normals.push_back(v);
        }
      } else if (lineEntries[0] == "vt") {
        vec2 uv;
        uv[0] = std::atof(lineEntries[1].c_str());
        uv[1] = std::atof(lineEntries[2].c_str());

        texCoords.push_back(uv);
      } else if (lineEntries[0] == "f") {
        // log("Face: {0}\n", line);

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
          vec3 tangent;
          vec3 bitangent;
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

std::vector<OBJMesh> meshes;

int activeMeshIndex = 0;

int parallaxLayers = 10;
float displacementScale = 0.0f;

bool useTexture = false;
bool useNormalTexture = false;
bool validNormalTexture = false;
bool useParallaxTexture = false;
bool useDisplacementMap = false;
bool enableTesselation = false;
bool useWireframe = false;

GLuint textureID = 0;
GLuint normalTextureID = 0;
GLuint parallaxTextureID = 0;
GLuint displacementMapID = 0;

vec3 tesselationOuter = vec3(2);
vec3 tesselationInner = vec3(2, 0, 0);

// TODO: update the vertex and fragment shaders to use Blinn-Phong shading.
void Lab08::init() {

  const char *Lab08VertexShaderSrc = R"VERTEXSHADER(
__VERSION__

uniform mat4 mvp;
uniform mat4 model;
uniform mat4 normalMatrix;

uniform sampler2D displacementMap;  // Added for displacement mapping.
uniform bool useDisplacementMap;
uniform float displacementScale;

// TODO(etagaca): Make this into a uniform.
float displacementBias = 0.0;

in vec3 vPosition;
in vec3 vNormal;
in vec2 texCoord;
in vec3 vTangent;
in vec3 vBitangent;

out vec3 fPos;
out vec3 fNormal;
out vec2 uv;

out mat3 TBN;

void main() {
	vec3 T = normalize(vec3(model * vec4(vTangent, 0.0)));
	vec3 B = normalize(vec3(model * vec4(vBitangent, 0.0)));
	vec3 N = (normalMatrix * vec4(vNormal, 0.0)).xyz;
	TBN = mat3(T, B, N);
	
	fPos = (model * vec4(vPosition, 1.0)).xyz;
	fNormal = N;
	uv = texCoord;

  vec3 temp = vPosition;

  if (useDisplacementMap) {
    vec4 color = texture(displacementMap, texCoord);
    vec3 newPosition = vPosition + N * (color.r * displacementScale + displacementBias);
    temp = newPosition;
  }

  gl_Position = mvp * vec4(temp, 1.0);
}

)VERTEXSHADER";

  // const char *tessControlShaderSrc = "";
  // const char* tessEvalShaderSrc = "";

  const char *tessControlShaderSrc = R"(
  __VERSION__
  uniform vec3 outerTesselation;
  uniform vec3 innerTesselation;

  layout(vertices = 3) out;

  in vec3 fPos[];
  in vec3 fNormal[];
  in vec2 uv[];
  in mat3 TBN[];

  out vec3 tc_fPos[];
  out vec3 tc_fNormal[];
  out vec2 tc_uv[];
  out mat3 tc_TBN[];

  void main() {
    tc_fPos[gl_InvocationID] = fPos[gl_InvocationID];
    tc_fNormal[gl_InvocationID] = fNormal[gl_InvocationID];
    tc_uv[gl_InvocationID] = uv[gl_InvocationID];
    tc_TBN[gl_InvocationID] = TBN[gl_InvocationID];

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    gl_TessLevelOuter[0] = outerTesselation[0];
    gl_TessLevelOuter[1] = outerTesselation[1];
    gl_TessLevelOuter[2] = outerTesselation[2];

    gl_TessLevelInner[0] = innerTesselation[0];
  }
)";

  const char *tessEvalShaderSrc = R"(
  __VERSION__
  uniform mat4 mvp;
  uniform sampler2D displacementMap;  // Added for displacement mapping.
  uniform bool useDisplacementMap;
  uniform float displacementScale;
  layout(triangles, equal_spacing, cw) in;

  in vec3 tc_fPos[];
  in vec3 tc_fNormal[];
  in vec2 tc_uv[];
  in mat3 tc_TBN[];

  out vec3 fPos;
  out vec3 fNormal;
  out vec2 uv;
  out mat3 TBN;

  int displacementBias = 0;

  void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    float w = gl_TessCoord.z;

    vec2 texCoord = tc_uv[0] * u + tc_uv[1] * v + tc_uv[2] * w;

    vec4 pos0 = gl_in[0].gl_Position;
    vec4 pos1 = gl_in[1].gl_Position;
    vec4 pos2 = gl_in[2].gl_Position;

    vec4 pos = u * pos0 + v * pos1 + w * pos2;

    if (useDisplacementMap) {
      vec4 color = texture(displacementMap, texCoord);
      
      // Use tc_fPos to get the tessellated vertex positions
      vec3 newPosition = tc_fPos[0] * u + tc_fPos[1] * v + tc_fPos[2] * w;

      // Displace along the normal direction
      newPosition += tc_fNormal[0] * (color.r * displacementScale + displacementBias);

      // Transform the displaced position to clip space
      gl_Position = mvp * vec4(newPosition, 1.0);
    } else {
      gl_Position = pos;
    }

    fPos = pos.xyz;
    fNormal = normalize(tc_TBN[0] * tc_fNormal[0] + tc_TBN[1] * tc_fNormal[1] + tc_TBN[2] * tc_fNormal[2]);
    uv = texCoord;
    TBN = tc_TBN[0] * u + tc_TBN[1] * v + tc_TBN[2] * w;
  }
)";

  const char *Lab08FragmentShaderSrc = R"FRAGMENTSHADER(
__VERSION__

uniform vec4 color;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;

uniform float Ia;
uniform vec3 Ka;

uniform sampler2D inputTexture;
uniform bool useTexture;

uniform sampler2D normalTexture;
uniform bool useNormalTexture;
uniform bool validNormalTexture;

uniform int parallaxLayers;
uniform bool useParallaxTexture;
uniform sampler2D heightmapTexture;  // Added for parallax mapping.

uniform bool useDisplacementMap;
uniform sampler2D displacementMap;
uniform float displacementScale;

// Diffuse
uniform float Id;
uniform vec3 Kd;

// Specular
uniform float shininess;
uniform vec3 Ks;

in vec3 fPos;
in vec3 fNormal;
in vec2 uv; // texture coordinates that is set in the vertex shader.
in mat3 TBN;

out vec4 FragColor;

vec2 uvCoord = uv;

float heightScale = 0.1f;  // Adjust value to control the intesity.

vec2 parallaxOcclusionMapping(vec2 textCoord, vec3 viewDir) {
  float height = texture(heightmapTexture, textCoord).r * heightScale;
  float layerHeight = height / parallaxLayers;
  vec2 deltaUV = viewDir.xy * height / viewDir.z / parallaxLayers;
  vec2 currentUV = textCoord;

  for (float i = 0.0; i < parallaxLayers; i++) {
    currentUV = currentUV - deltaUV;
    float currentHeight = texture(heightmapTexture, currentUV).r * heightScale;

    // Adjust the threshold for smoother transitions
    // TODO(etagaca): Make this into a uniform.
    float threshold = 0.01;
    if (currentHeight > height + threshold) {
      return currentUV;
    }
  }
  return textCoord;
}

void main() {

	vec3 normal;
  vec3 viewDirection = normalize(cameraPosition - fPos);

  if (useParallaxTexture) {
    uvCoord = parallaxOcclusionMapping(uvCoord, viewDirection);
  }

	if (useNormalTexture && validNormalTexture) {
		normal = texture(normalTexture, uvCoord).rgb;
		normal = normal * 2.0 - 1.0;
		normal = normalize(TBN * normal);
	}
	else {
		normal = normalize(fNormal);
	}
	
	// Normal in range of [-1, 1] for all of the xyz values. 
	// Remap the normal to go into the range of [0, 1] for xyz
	//vec3 normalColor = (fNormal + vec3(1)) * 0.5;
	vec3 ambientTerm = Ia * Ka;
	vec3 diffuseTerm = Id * max(0.0, dot(normal, -lightDirection)) * Kd;

	vec3 R = reflect(lightDirection, normal);

	// Specular terms for Phong
	vec3 specularTerm = pow(max(0.0, dot(R, viewDirection)), shininess) * Ks;

	// Specular term for Blinn-Phong
	vec3 halfway = normalize(-lightDirection + viewDirection);
	specularTerm = pow(max(0.0, dot(halfway, normal)), shininess) * Ks;

	vec3 finalColor = ambientTerm + diffuseTerm + specularTerm;

	vec4 texColor = vec4(1.0);

	if (useTexture) {
    texColor = texture(inputTexture, uvCoord);
	}

	FragColor = vec4(finalColor * texColor.rgb, 1.0);
}

)FRAGMENTSHADER";

  if (enableTesselation) {
    if (!shader.init(Lab08VertexShaderSrc, Lab08FragmentShaderSrc,
                     tessControlShaderSrc, tessEvalShaderSrc)) {
      exit(1);
    }
  } else {
    if (!shader.init(Lab08VertexShaderSrc, Lab08FragmentShaderSrc)) {
      exit(1);
    }
  }

  // Default sphere mesh
  OBJMesh sphereMesh = OBJMesh::getSphere(shader.program);
  meshes.push_back(sphereMesh);

  addSceneObject();

  initialized = true;
}

// Renders to the "screen" texture that has been passed in as a parameter
void Lab08::render(s_ptr<Framebuffer> framebuffer) {
  if (!initialized)
    init();

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  if (useWireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_CULL_FACE);
  }

  double deltaTime = Application::get().deltaTime;
  double timeSinceStart = Application::get().timeSinceStart;

  glUseProgram(shader.program);

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

    // TODO: update uniform variables in shader required for Blinn-Phong
    // shading. Ambient light intensity and color are already being passed in.
    GLint iaLocation = glGetUniformLocation(shader.program, "Ia");
    glUniform1f(iaLocation, light.Ia);

    GLint kaLocation = glGetUniformLocation(shader.program, "Ka");
    glUniform3fv(kaLocation, 1, glm::value_ptr(light.Ka));

    // Diffuse
    GLint idLocation = glGetUniformLocation(shader.program, "Id");
    glUniform1f(idLocation, light.Id);

    GLint kdLocation = glGetUniformLocation(shader.program, "Kd");
    glUniform3fv(kdLocation, 1, glm::value_ptr(light.Kd));

    // Specular
    GLint shineLocation = glGetUniformLocation(shader.program, "shininess");
    glUniform1f(shineLocation, light.shininess);

    GLint ksLocation = glGetUniformLocation(shader.program, "Ks");
    glUniform3fv(ksLocation, 1, glm::value_ptr(light.Ks));

    GLint lightDirLocation =
        glGetUniformLocation(shader.program, "lightDirection");
    glUniform3fv(lightDirLocation, 1, glm::value_ptr(light.lightDirection));

    GLint camPosLocation =
        glGetUniformLocation(shader.program, "cameraPosition");
    glUniform3fv(camPosLocation, 1, glm::value_ptr(camera.cameraPosition));

    GLint colorLocation = glGetUniformLocation(shader.program, "color");
    glUniform4fv(colorLocation, 1, glm::value_ptr(sceneObject.color));

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
      log("normal texture valid: {0}\n", validNormalTexture);

      if (OpenGLRenderer::instance->textures.find(normalTextureID) !=
          OpenGLRenderer::instance->textures.end()) {
        auto normalTexture =
            OpenGLRenderer::instance->textures[normalTextureID];
        log("normal Texture FOUND!!!!\n");
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

        // Get the location of the normal map uniform .
        GLint normalTextureSamplerLocation =
            glGetUniformLocation(shader.program, "normalTexture");

        // Set the texture unit index (e.g., 1) to the normal map uniform.
        glUniform1i(normalTextureSamplerLocation, 1);
      } else {
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

        log("parallax texture id: {0}\n", parallaxTextureID);
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

    GLint useTexLocation = glGetUniformLocation(shader.program, "useTexture");
    glUniform1i(useTexLocation, (GLint)useTexture);

    // Checker for valid normal texture id.
    GLint validNormalTextureLocation =
        glGetUniformLocation(shader.program, "validNormalTexture");
    glUniform1i(validNormalTextureLocation, (GLint)validNormalTexture);

    GLint useNormalTexLocation =
        glGetUniformLocation(shader.program, "useNormalTexture");
    glUniform1i(useNormalTexLocation, (GLint)useNormalTexture);

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

    if (enableTesselation) {
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
}

void Lab08::renderUI() {

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

  ImGui::Checkbox("Use parallax texture", &useParallaxTexture);
  if (useParallaxTexture) {
    int texID = parallaxTextureID;
    if (ImGui::InputInt("Parallax Texture ID", &texID)) {
      parallaxTextureID = texID;
    }
    ImGui::SliderInt("Parallax layers", &parallaxLayers, 0, 100);
  }

  ImGui::Checkbox("Use displacement map", &useDisplacementMap);
  if (useDisplacementMap) {
    int texID = displacementMapID;
    if (ImGui::InputInt("Displacement Map ID", &texID)) {
      displacementMapID = texID;
    }
    ImGui::SliderFloat("Displacement scale", &displacementScale, 0.0f, 1.0f);
  }

  ImGui::Checkbox("Enable Tesselation", &enableTesselation);
  int tesselationLevelOuter = tesselationOuter.x;
  int tesselationLevelInner = tesselationInner.x;
  if (enableTesselation) {
    if (ImGui::SliderInt("Tesselation level outer", &tesselationLevelOuter, 2,
                         100)) {
      tesselationOuter = vec3(tesselationLevelOuter);
    }
    if (ImGui::SliderInt("Tesselation level inner", &tesselationLevelInner, 2,
                         100)) {
      tesselationInner = vec3(tesselationLevelInner);
    }
  }

  ImGui::Checkbox("Wireframe", &useWireframe);

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
