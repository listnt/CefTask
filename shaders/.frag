#version 300 es
precision mediump float;

in vec4 color;
in vec2 uv;

out vec4 fragColor;

uniform sampler2D browserImg;

uniform sampler2D y_texture; 
uniform sampler2D u_texture;
uniform sampler2D v_texture;


uniform int mode; // 0 for rgba, 1 for yuv

void main() {
  // fragColor = color;

  vec2 flippedUV = vec2(uv.x, 1.0 - uv.y);

  if (mode == 0) {
    fragColor = texture2D(browserImg, flippedUV);
  } else {

    float y = texture2D(y_texture, flippedUV).r;
    float u = texture2D(u_texture, flippedUV).r - 0.5;
    float v = texture2D(v_texture, flippedUV).r - 0.5;

    float r = y + 1.40200 * v;
    float g = y - 0.34414 * u - 0.71414 * v;
    float b = y + 1.77200 * u;

    fragColor = vec4(r, g, b, 1.0);
  }
}