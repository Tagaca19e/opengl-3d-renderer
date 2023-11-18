__VERSION__

uniform mat4 mvp;
uniform mat4 model;
uniform mat4 normalMatrix;

uniform sampler2D displacementMap;
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
    vec3 newPosition =
        vPosition + N * (color.r * displacementScale + displacementBias);
    temp = newPosition;
  }

  gl_Position = mvp * vec4(temp, 1.0);
}
