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
uniform sampler2D heightmapTexture; // Added for parallax mapping.

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

float heightScale = 0.1f; // Adjust value to control the intesity.

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
  } else {
    normal = normalize(fNormal);
  }

  // Normal in range of [-1, 1] for all of the xyz values.
  // Remap the normal to go into the range of [0, 1] for xyz
  // vec3 normalColor = (fNormal + vec3(1)) * 0.5;
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
