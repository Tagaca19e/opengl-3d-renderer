__VERSION__

out vec4 FragColor;

in vec3 texCoords;

uniform samplerCube cubemap;

void main() {
  FragColor = texture(cubemap, texCoords);
}
