#version 460

#include "../Camera.lib.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 inNormal;

layout(set = 0, binding = 0) uniform CameraUBO {
    Camera cam;
};

layout(set = 1, binding = 0) uniform MaterialUBO {
    vec4 color;
} mat;

void main() {
    outColor = mat.color;
}
