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
