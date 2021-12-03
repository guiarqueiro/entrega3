#version 410

layout(location = 0) in vec3 inPosition;

uniform vec4 color;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out vec4 fragColor;

void main() {
  vec4 posEyeSpace = viewMatrix * modelMatrix * vec4(inPosition, 1);
  float i = 1.0 - (-posEyeSpace.z / 100.0);
  fragColor = vec4(i, i, i, 1) * color;
  /*fragColor = vec4(inPosition.x, 1, inPosition.y, 1);
  fragColor = vec4(posEyeSpace.x, 1, posEyeSpace.y, 1);*/
  gl_Position = projMatrix * posEyeSpace;
}