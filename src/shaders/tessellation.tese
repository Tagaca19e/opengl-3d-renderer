__VERSION__

uniform mat4 mvp;
uniform sampler2D displacementMap; // Added for displacement mapping.
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
    newPosition +=
        tc_fNormal[0] * (color.r * displacementScale + displacementBias);

    // Transform the displaced position to clip space
    gl_Position = mvp * vec4(newPosition, 1.0);
  } else {
    gl_Position = pos;
  }

  fPos = pos.xyz;
  fNormal = normalize(tc_TBN[0] * tc_fNormal[0] + tc_TBN[1] * tc_fNormal[1] +
                      tc_TBN[2] * tc_fNormal[2]);
  uv = texCoord;
  TBN = tc_TBN[0] * u + tc_TBN[1] * v + tc_TBN[2] * w;
}
