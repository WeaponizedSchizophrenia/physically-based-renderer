#version 460

#include "../Camera.lib.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;

layout(push_constant) uniform PC {
  mat4x4 model;
  mat3x3 normalModel;
} pc;

layout(set = 0, binding = 0) uniform CameraUBO {
  Camera cam;
};

void main() {
    gl_Position = cam.proj * cam.view * pc.model * vec4(inPosition, 1.0);
    outNormal = pc.normalModel * inNormal;
}
