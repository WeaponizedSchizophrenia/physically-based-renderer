#version 460

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 inColor;

layout(push_constant, std430) uniform pc {
  vec3 color;
  float mixFactor;
};

void main() {
    outColor = vec4(mix(inColor, color, mixFactor), 1.0);
}
