#version 300 es
precision mediump float;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec2 vUV;

uniform mat4 viewModel;
uniform mat4 projection;

out vec4 color;
out vec2 uv;

void main() {
  color = vColor;
  uv = vUV;
  gl_Position = projection * viewModel * vec4(vPosition.xyz, 1.0);
}
