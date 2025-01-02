#version 460

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 inNormal;

layout(set = 0, binding = 0) uniform CameraData {
    mat4x4 view;
    mat4x4 proj;
    vec3 position;
} cam;

layout(set = 1, binding = 0) uniform Material {
    vec4 color;
} mat;

void main() {
    outColor = mat.color;
}
