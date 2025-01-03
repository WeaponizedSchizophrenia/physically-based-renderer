#version 460

#include "../Camera.lib.glsl"
#include "../BlinnPhong.lib.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(set = 0, binding = 0) uniform CameraUBO {
    Camera cam;
};

layout(set = 1, binding = 0) uniform MaterialUBO {
    vec4 color;
} mat;

void main() {
    vec3 V = normalize(cam.position - inPosition);
    vec3 L = V;
    outColor = blinnPhongLighting(mat.color, inNormal, V, L);
}
